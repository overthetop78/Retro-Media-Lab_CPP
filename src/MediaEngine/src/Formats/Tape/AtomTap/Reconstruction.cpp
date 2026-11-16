#include "Reader.h"
#include "Constants.h"
#include "../Uef/Constants.h"
#include "../../../Sources/RandomAccessSources.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <QString>
#include <algorithm>
#include <bit>
namespace RetroMediaLab::MediaEngine::AtomTap {
using namespace Constants;
std::shared_ptr<const MediaDocument> Reader::readBytes(std::shared_ptr<const MediaSource> source,std::span<const std::uint8_t> bytes,std::stop_token token){
    if(!validate(bytes,token))throw MediaError(DiagnosticCode::AtomTapData);std::vector<SequentialSegment> segments;
    const auto add=[&](std::uint16_t id,std::vector<std::uint8_t> payload,SequentialSegmentKind kind){SequentialSegmentFields value;value.position=segments.size();value.kind=kind;value.dataRange=DataRange(0,payload.size(),DataRangeKind::Stored,memorySource(payload));value.metadata={{std::string(Uef::Constants::ChunkIdMetadataKey),QString::number(id,HexRadix).rightJustified(HexWidth,DigitZero).toStdString()},{std::string(Uef::Constants::BaseFrequencyMetadataKey),std::to_string(Uef::Constants::DefaultBaseFrequency)},{std::string(Uef::Constants::BaudRateMetadataKey),std::to_string(Baud)},{std::string(Uef::Constants::PhaseMetadataKey),std::to_string(InitialPhase)}};segments.emplace_back(std::move(value));};
    const auto word=[&](std::uint16_t id,int value,SequentialSegmentKind kind){std::vector<std::uint8_t> payload(WordBytes);qToLittleEndian(static_cast<std::uint16_t>(value),payload.data());add(id,std::move(payload),kind);};
    const auto data=[&](std::vector<std::uint8_t> payload){payload.insert(payload.begin(),{DataBits,NoParity,StopBits});add(Uef::Constants::DefinedData,std::move(payload),SequentialSegmentKind::DataBlock);};
    word(Uef::Constants::BaudRate,Baud,SequentialSegmentKind::TapeMark);std::size_t offset{};
    while(offset<bytes.size()){checkCancellation(token);const auto file=bytes.subspan(offset,FileHeaderBytes);const auto end=std::find(file.begin(),file.begin()+FileNameBytes,0);std::vector<std::uint8_t> name(file.begin(),end);const auto load=qFromLittleEndian<std::uint16_t>(file.data()+LoadAddressOffset),execute=qFromLittleEndian<std::uint16_t>(file.data()+ExecutionAddressOffset),length=qFromLittleEndian<std::uint16_t>(file.data()+LengthOffset);offset+=FileHeaderBytes;int copied=0,number=0,history=((load+length)>>EndAddressShift)&FlagHistoryMask;
        do{checkCancellation(token);const auto count=std::min(MaximumBlockBytes,length-copied);const auto flags=static_cast<std::uint8_t>(history|(count>0?ContainsData:0)|(copied+count<length?MoreBlocks:0)|(number>0?Continuation:0));std::vector<std::uint8_t> header(SyncBytes,Sync);header.insert(header.end(),name.begin(),name.end());header.push_back(NameTerminator);header.push_back(flags);const auto big=[&](int value){header.push_back(static_cast<std::uint8_t>(value>>DataBits));header.push_back(static_cast<std::uint8_t>(value));};big(number);header.push_back(static_cast<std::uint8_t>(count-1));big(execute);big(load+copied);std::vector<std::uint8_t> payload(bytes.begin()+offset+copied,bytes.begin()+offset+copied+count);unsigned checksum{};for(auto value:header)checksum+=value;for(auto value:payload)checksum+=value;payload.push_back(static_cast<std::uint8_t>(checksum));
            word(Uef::Constants::CarrierTone,CarrierCycles,SequentialSegmentKind::Carrier);data(std::move(header));word(Uef::Constants::CarrierTone,HeaderDataCarrierCycles,SequentialSegmentKind::Carrier);data(std::move(payload));std::vector<std::uint8_t> gap(FloatBytes);qToLittleEndian(std::bit_cast<std::uint32_t>(static_cast<float>(GapSeconds)),gap.data());add(Uef::Constants::FloatingGap,std::move(gap),SequentialSegmentKind::Silence);copied+=count;++number;history=(flags>>FlagHistoryShift)&FlagHistoryMask;
        }while(copied<length);offset+=length;
    }
    return std::make_shared<const MediaDocument>(std::move(source),std::string(FormatId),MediaKind::Tape,std::make_shared<SequentialImage>(bytes.size(),std::nullopt,std::move(segments)));
}
}
