#include "Encoder.h"
#include "../../../Primitives/Bits.h"
#include "../../../Formats/Tape/Amstrad/Constants.h"
#include "../../../Formats/Tape/Amstrad/Records.h"
#include "../../../Formats/Tape/Tzx/Constants.h"
#include "../../../Formats/Tape/Tzx/Metadata.h"
#include "../../../Formats/Tape/Common/Pcm.h"
#include "../../../Sources/RandomAccessSources.h"
#include "../../../Images/RecognitionContext.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QString>
#include <QtEndian>
#include <charconv>
#include <algorithm>
#include <limits>
#include <cmath>
namespace RetroMediaLab::MediaEngine {
namespace {
using namespace AmstradTape::Constants;
using Metadata=std::map<std::string,std::string>;
int integer(const Metadata& values,std::string_view key,int fallback,int maximum){
    const auto found=values.find(std::string(key));if(found==values.end())return fallback;unsigned value{};const auto& text=found->second;const auto parsed=std::from_chars(text.data(),text.data()+text.size(),value,DecimalRadix);
    return parsed.ec==std::errc{}&&parsed.ptr==text.data()+text.size()&&value<=static_cast<unsigned>(maximum)?static_cast<int>(value):fallback;
}
bool boolean(const Metadata& values,std::string_view key,bool fallback){
    const auto found=values.find(std::string(key));if(found==values.end())return fallback;const auto text=QString::fromUtf8(found->second).trimmed().toStdString();if(sameIdentifier(text,TrueValue))return true;if(sameIdentifier(text,FalseValue))return false;return fallback;
}
std::string name(const SequentialDecodedBlock& block,std::size_t index){const auto found=block.metadata().find(std::string(FileNameKey));return found!=block.metadata().end()&&!QString::fromUtf8(found->second).trimmed().isEmpty()?found->second:std::string(DefaultNamePrefix)+QString::number(index+1).rightJustified(NameDigits,QLatin1Char(DigitZero)).toStdString();}
bool sameFile(const SequentialDecodedBlock& block,const std::string& fileName){const auto found=block.metadata().find(std::string(FileNameKey));return found!=block.metadata().end()&&found->second==fileName;}
std::vector<std::vector<std::uint8_t>> records(std::span<const SequentialDecodedBlock> blocks,std::stop_token token){
    std::vector<std::vector<std::uint8_t>> result;result.reserve(blocks.size()*PulsePair);
    for(std::size_t index=0;index<blocks.size();++index){
        checkCancellation(token);const auto& block=blocks[index];const auto bytes=block.data();const auto fileName=name(block,index);const bool first=index==0||!sameFile(blocks[index-1],fileName),last=index+1==blocks.size()||!sameFile(blocks[index+1],fileName);
        std::vector<std::uint8_t> header(SegmentDataLength);const auto latin=QString::fromUtf8(fileName).toLatin1();std::copy_n(latin.begin(),std::min<std::size_t>(latin.size(),FileNameLength),header.begin()+FileNameOffset);const auto& meta=block.metadata();
        header[BlockNumberOffset]=static_cast<std::uint8_t>(integer(meta,BlockNumberKey,static_cast<int>(index+1),MaximumByte));header[LastBlockOffset]=boolean(meta,LastBlockKey,last)?TrailerByte:0;header[FileTypeOffset]=static_cast<std::uint8_t>(integer(meta,FileTypeKey,DefaultFileType,MaximumByte));qToLittleEndian(static_cast<std::uint16_t>(bytes.size()),header.data()+DataLengthOffset);
        qToLittleEndian(static_cast<std::uint16_t>(integer(meta,DataLocationKey,0,std::numeric_limits<std::uint16_t>::max())),header.data()+DataLocationOffset);header[FirstBlockOffset]=boolean(meta,FirstBlockKey,first)?TrailerByte:0;
        qToLittleEndian(static_cast<std::uint16_t>(integer(meta,LogicalLengthKey,static_cast<int>(bytes.size()),std::numeric_limits<std::uint16_t>::max())),header.data()+LogicalLengthOffset);qToLittleEndian(static_cast<std::uint16_t>(integer(meta,EntryAddressKey,0,std::numeric_limits<std::uint16_t>::max())),header.data()+EntryAddressOffset);
        result.push_back(AmstradTape::encodeRecord(HeaderSync,header,token));result.push_back(AmstradTape::encodeRecord(DataSync,bytes,token));
    }
    return result;
}
std::shared_ptr<const SequentialImage> tzx(const std::vector<std::vector<std::uint8_t>>& records,std::stop_token token){
    std::vector<SequentialSegment> output;std::int64_t length=0;
    for(const auto& record:records){
        checkCancellation(token);using namespace Tzx::Constants;std::vector<std::uint8_t> payload(TurboHeaderSize+record.size());
        qToLittleEndian(OnePulseTStates,payload.data());qToLittleEndian(ZeroPulseTStates,payload.data()+TurboSyncFirstOffset);qToLittleEndian(ZeroPulseTStates,payload.data()+TurboSyncSecondOffset);qToLittleEndian(ZeroPulseTStates,payload.data()+TurboZeroOffset);qToLittleEndian(OnePulseTStates,payload.data()+TurboOneOffset);qToLittleEndian(PilotPulseCount,payload.data()+TurboPilotCountOffset);payload[TurboUsedBitsOffset]=UsedBitsInLastByte;qToLittleEndian(PauseMilliseconds,payload.data()+TurboPauseOffset);
        Primitives::writeLittle24(payload,TurboLengthOffset,static_cast<std::uint32_t>(record.size()));std::copy(record.begin(),record.end(),payload.begin()+TurboHeaderSize);
        SequentialSegmentFields value;value.position=output.size();value.kind=SequentialSegmentKind::DataBlock;value.length=payload.size();value.dataRange=DataRange(0,payload.size(),DataRangeKind::Stored,memorySource(payload));value.metadata={{std::string(BlockIdMetadataKey),Tzx::blockIdText(TurboSpeedData)},{std::string(PayloadKey),std::to_string(payload.size())},{std::string(KnownKey),std::string(AmstradTape::Constants::TrueValue)},{std::string(ReconstructionKey),std::string(AmstradTape::Constants::TrueValue)}};
        length+=payload.size();output.emplace_back(std::move(value));
    }
    return std::make_shared<const SequentialImage>(length,std::nullopt,std::move(output));
}
std::shared_ptr<const SequentialImage> pcm(const std::vector<std::vector<std::uint8_t>>& records,int rate,std::stop_token token){
    std::size_t count=0;const auto measure=[&](std::uint16_t states,int pulses){const auto samples=std::max(double(1),TapePcm::roundEven(double(states)*rate/Tzx::Constants::TStatesPerSecond))*pulses;if(!std::isfinite(samples)||samples>MaximumPcmSamples-count)throw MediaError(DiagnosticCode::CpcTapeLimit);count+=static_cast<std::size_t>(samples);};
    for(const auto& record:records){checkCancellation(token);if(static_cast<std::size_t>(rate)>MaximumPcmSamples-count)throw MediaError(DiagnosticCode::CpcTapeLimit);count+=rate;measure(OnePulseTStates,PilotPulseCount);measure(ZeroPulseTStates,PulsePair);for(auto value:record)for(int bit=LastBit;bit>=0;--bit)measure((value&(1<<bit))?OnePulseTStates:ZeroPulseTStates,PulsePair);}
    if(static_cast<std::size_t>(rate)>MaximumPcmSamples-count)throw MediaError(DiagnosticCode::CpcTapeLimit);count+=rate;std::vector<std::int16_t> samples;samples.reserve(count);auto level=PcmAmplitude;
    const auto append=[&](std::uint16_t states,int pulses){checkCancellation(token);TapePcm::appendPulses(samples,level,states,pulses,rate,Tzx::Constants::TStatesPerSecond);};
    for(const auto& record:records){TapePcm::appendSilence(samples,rate,1);append(OnePulseTStates,PilotPulseCount);append(ZeroPulseTStates,PulsePair);for(auto value:record)for(int bit=LastBit;bit>=0;--bit)append((value&(1<<bit))?OnePulseTStates:ZeroPulseTStates,PulsePair);}TapePcm::appendSilence(samples,rate,1);
    SourceOptions options;options.primaryPath=EncodedWave;const auto document=TapePcm::signalDocument(std::make_shared<MediaSource>(std::move(options)),std::string(WaveFormat),samples,rate,token);const auto& image=dynamic_cast<const SequentialImage&>(document->representation());auto value=image.segments()->front().fields();value.length=samples.size();value.start=SequentialTime{};value.duration=image.duration();value.direction=SequentialTravelDirection::Forward;value.metadata[std::string(ChannelKey)]=ChannelZero;value.metadata[std::string(InterleavedKey)]=TrueValue;value.metadata[std::string(ReconstructionKey)]=TrueValue;
    return std::make_shared<const SequentialImage>(image.logicalLength(),image.duration(),std::vector<SequentialSegment>{SequentialSegment(std::move(value))});
}
}
std::string_view AmstradCpcTapeEncoder::id()const noexcept{return EncoderId;}
std::span<const std::string> AmstradCpcTapeEncoder::formatIds()const{static const std::vector<std::string> ids{std::string(Tzx::Constants::FormatId),std::string(WaveFormat),std::string(VocFormat)};return ids;}
std::span<const std::string> AmstradCpcTapeEncoder::machineIds()const{static const std::vector<std::string> ids{std::string(MachineId)};return ids;}
bool AmstradCpcTapeEncoder::canEncode(const SequentialEncodeRequest& request)const{
    return std::any_of(formatIds().begin(),formatIds().end(),[&](const auto& id){return sameIdentifier(request.targetFormatId(),id);})&&sameIdentifier(request.machineId(),MachineId)&&!request.blocks().empty()&&request.blocks().size()<=MaximumByte&&request.retainedSegments().empty()&&std::all_of(request.blocks().begin(),request.blocks().end(),[](const auto& block){return !block.data().empty()&&block.data().size()<=MaximumDataLength;});
}
std::future<std::shared_ptr<const SequentialImage>> AmstradCpcTapeEncoder::encodeAsync(SequentialEncodeRequest request,std::stop_token token)const{return std::async(std::launch::deferred,[request=std::move(request),token]{
    checkCancellation(token);if(!AmstradCpcTapeEncoder{}.canEncode(request))throw MediaError(DiagnosticCode::CpcTapeEncode);const auto encoded=records(request.blocks(),token);if(sameIdentifier(request.targetFormatId(),Tzx::Constants::FormatId))return tzx(encoded,token);int rate{};if(!TapePcm::trySampleRate(request.parameters(),rate))rate=DefaultSampleRate;return pcm(encoded,rate,token);
});}
}
