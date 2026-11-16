#include "Internal.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QString>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::HxcStream {
namespace {
using namespace Constants;
std::int32_t exactResolution(const Flux::ProtectedTrack& track) {
    for(const auto resolution:Resolutions){bool valid=true;
        for(const auto& revolution:track.revolutions()){
            if(std::uint64_t(revolution.flux().indexTimeTicks)*revolution.resolutionNanoseconds()%(std::uint32_t(resolution)*IoSampleTicks)!=0){valid=false;break;}
            for(const auto interval:revolution.flux().intervals)if(std::uint64_t(interval)*revolution.resolutionNanoseconds()%std::uint32_t(resolution)!=0){valid=false;break;}
            if(!valid)break;
        }if(valid)return resolution;
    }throw MediaError(DiagnosticCode::HxcResolution);
}
std::uint64_t addTime(std::uint64_t a,std::uint64_t b){if(b>MaximumTime-a)throw MediaError(DiagnosticCode::HxcIndexPeriod);return a+b;}
bool validField(std::string_view key,std::string_view value){
    if(key.empty()||key.find(Null)!=std::string_view::npos||value.find(Null)!=std::string_view::npos||value.find(Newline)!=std::string_view::npos||value.find(CarriageReturn)!=std::string_view::npos)return false;
    const auto text=QString::fromUtf8(key.data(),qsizetype(key.size()));return std::none_of(text.begin(),text.end(),[](QChar c){return c.isSpace();});
}
}
bool canEncode(const Flux::ProtectedTrack& track){
    if(track.revolutions().empty())return false;try{return exactResolution(track)>0;}catch(const MediaError& e){if(e.diagnostic().code==DiagnosticCode::HxcResolution)return false;throw;}
}
std::vector<std::uint8_t> encode(const Flux::ProtectedTrack& track,bool writeProtected,const Metadata& metadata,std::stop_token token) {
    using namespace Constants;checkCancellation(token);if(track.revolutions().empty())throw MediaError(DiagnosticCode::HxcNoRevolutions);
    const auto resolution=std::uint32_t(exactResolution(track));std::vector<std::uint64_t> indexes{0};std::vector<std::uint32_t> pulses;std::uint64_t elapsed{},previous{};
    for(const auto& revolution:track.revolutions()){
        checkCancellation(token);std::uint64_t position{};
        for(const auto interval:revolution.flux().intervals){checkCancellation(token);position=addTime(position,std::uint64_t(interval)*revolution.resolutionNanoseconds()/resolution);const auto absolute=addTime(elapsed,position);
            if(absolute<=previous||absolute-previous>MaximumPulse)throw MediaError(DiagnosticCode::HxcPulseInterval);
            pulses.push_back(std::uint32_t(absolute-previous));previous=absolute;
        }
        const auto duration=std::uint64_t(revolution.flux().indexTimeTicks)*revolution.resolutionNanoseconds()/resolution;
        if(position>duration||duration==0)throw MediaError(DiagnosticCode::HxcIndexPeriod);
        elapsed=addTime(elapsed,duration);indexes.push_back(elapsed);
    }
    const auto samples=elapsed/IoSampleTicks+TrailingIoSamples;if(samples>MaximumUnpackedBytes/WordBytes)throw MediaError(DiagnosticCode::HxcIoLength);
    std::vector<std::uint8_t> io(std::size_t(samples*WordBytes));
    if(writeProtected)for(std::size_t at=0;at<io.size();at+=WordBytes)io[at]=WriteProtectedFlag;
    for(const auto index:indexes)io[std::size_t(index/IoSampleTicks*WordBytes)]|=IndexFlag;
    std::vector<std::uint8_t> packedPulses;for(const auto pulse:pulses){checkCancellation(token);appendPulse(packedPulses,pulse);}
    const auto prefix=fieldsPrefix(track.cylinder(),track.head());std::string info=std::string(SampleRateKey)+Space+std::to_string(NanosecondsPerSecond/resolution)+Newline;bool first=true;
    for(const auto& [key,value]:metadata){if(!key.starts_with(prefix))continue;const auto field=std::string_view(key).substr(prefix.size());if(field==SampleRateKey||!validField(field,value))continue;
        if(!first)info+=Newline;first=false;info+=std::string(field)+Space+value;
    }info+=Null;
    std::vector<std::uint8_t> bytes(ChunkHeaderBytes);appendBlock(bytes,MetadataBlock,{reinterpret_cast<const std::uint8_t*>(info.data()),info.size()});appendBlock(bytes,PackedIoBlock,io);
    if(!pulses.empty()){if(pulses.size()>MaximumSignedSize)throw MediaError(DiagnosticCode::HxcChunkLength);appendBlock(bytes,PackedStreamBlock,packedPulses,std::uint32_t(pulses.size()));}
    const auto length=bytes.size()+CrcBytes;if(length>MaximumChunkBytes)throw MediaError(DiagnosticCode::HxcChunkLength);
    for(std::size_t i=0;i<FieldBytes;++i){bytes[i]=std::uint8_t(ChunkSignature>>(i*ByteBits));bytes[ChunkSizeOffset+i]=std::uint8_t(length>>(i*ByteBits));}
    appendLittleEndian(bytes,checksum(bytes,CrcSeed));return bytes;
}
}
