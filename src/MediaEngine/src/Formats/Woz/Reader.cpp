#include "Woz.h"
#include "Constants.h"
#include "../Apple/DecodedTracks.h"
#include "../Apple/TrackSelectionConstants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <array>
#include <map>
namespace RetroMediaLab::MediaEngine::Woz {
namespace {
using namespace Constants;
using Bytes=MediaFileSystems::Bytes;
std::uint32_t little(Bytes data,std::size_t offset,std::size_t size){std::uint32_t value=0;for(std::size_t i=0;i<size;++i)value|=std::uint32_t(data[offset+i])<<(i*BitsPerByte);return value;}
bool starts(Bytes data,std::string_view signature){return data.size()>=signature.size()&&std::equal(signature.begin(),signature.end(),data.begin());}
Version validate(Bytes data){
    if(data.size()<MinimumFileLength||!std::equal(Marker.begin(),Marker.end(),data.begin()+MarkerOffset))throw MediaError(DiagnosticCode::WozHeader);
    const auto version=starts(data,Version1Signature)?Version::Woz1:starts(data,Version2Signature)?Version::Woz2:throw MediaError(DiagnosticCode::WozHeader);
    const auto stored=little(data,CrcOffset,CrcLength),computed=crc32(data.subspan(ChunksOffset));
    if(stored!=computed)throw MediaError(DiagnosticCode::WozCrc,{std::to_string(stored),std::to_string(computed)});return version;
}
std::map<std::string,Bytes> chunks(Bytes data){
    std::map<std::string,Bytes> result;std::size_t offset=ChunksOffset;
    while(offset<=data.size()-ChunkHeaderLength){
        std::string id;for(std::size_t i=0;i<ChunkIdLength;++i)id.push_back(char(data[offset+i]<=AsciiMaximum?data[offset+i]:AsciiReplacement));
        const auto length=little(data,offset+ChunkLengthOffset,ChunkLengthSize);offset+=ChunkHeaderLength;
        if(length>MaximumLength||length>data.size()-offset)throw MediaError(DiagnosticCode::WozTruncatedChunk,{id});
        result[id]=data.subspan(offset,length);offset+=length;
    }
    return result;
}
Bytes required(const std::map<std::string,Bytes>& chunks,std::string_view id,std::size_t minimum){
    const auto found=chunks.find(std::string(id));if(found==chunks.end()||found->second.size()<minimum)throw MediaError(DiagnosticCode::WozMissingChunk,{std::string(id)});return found->second;
}
[[noreturn]] void referenceError(std::int32_t track,std::uint8_t descriptor){throw MediaError(DiagnosticCode::WozTrackReference,{std::to_string(track),std::to_string(descriptor)});}
Flux::Bits unpack(Bytes data,std::size_t count){Flux::Bits bits(count);for(std::size_t i=0;i<count;++i)bits[i]=std::uint8_t((data[i/BitsPerByte]>>(BitsPerByte-1-i%BitsPerByte))&1);return bits;}
Flux::Bits trackBits(Bytes file,Bytes tracks,Version version,std::int32_t track,std::uint8_t descriptor){
    if(version==Version::Woz1){
        const auto offset=std::size_t(descriptor)*Woz1TrackEntryLength;
        if(offset>tracks.size()||Woz1TrackEntryLength>tracks.size()-offset)referenceError(track,descriptor);
        const auto count=little(tracks,offset+Woz1BitCountOffset,Woz1BitCountLength);
        if(count==0||count>Woz1MaximumBitCount)return {};
        return unpack(tracks.subspan(offset,(count+BitsPerByte-1)/BitsPerByte),count);
    }
    const auto descriptorOffset=std::size_t(descriptor)*Woz2DescriptorLength;
    if(descriptorOffset>tracks.size()||Woz2DescriptorLength>tracks.size()-descriptorOffset)referenceError(track,descriptor);
    const auto start=little(tracks,descriptorOffset+Woz2StartBlockOffset,Woz2BlockFieldLength),blocks=little(tracks,descriptorOffset+Woz2BlockCountOffset,Woz2BlockFieldLength),count=little(tracks,descriptorOffset+Woz2BitCountOffset,Woz2BitCountLength);
    if(count>MaximumLength)referenceError(track,descriptor);
    const auto offset=std::size_t(start)*Woz2BlockLength,bytes=(std::size_t(count)+BitsPerByte-1)/BitsPerByte;
    if(start==0||blocks==0||count==0||bytes>std::size_t(blocks)*Woz2BlockLength||offset>file.size()||bytes>file.size()-offset)referenceError(track,descriptor);
    return unpack(file.subspan(offset,bytes),count);
}
}
std::uint32_t crc32(MediaFileSystems::Bytes bytes) noexcept{
    auto crc=std::numeric_limits<std::uint32_t>::max();for(const auto value:bytes){crc^=value;for(std::size_t bit=0;bit<Constants::BitsPerByte;++bit)crc=(crc>>1)^((crc&1)?Constants::CrcPolynomial:0);}return ~crc;
}
Tracks readTracks(MediaFileSystems::Bytes data,std::stop_token token){
    checkCancellation(token);const auto version=validate(data);const auto table=chunks(data);const auto info=required(table,InfoId,MinimumInfoLength);
    if(info[InfoDiskTypeOffset]!=AppleDiskType)throw MediaError(DiagnosticCode::WozDiskType,{std::to_string(info[InfoDiskTypeOffset])});
    const auto map=required(table,TrackMapId,MapLength),tracks=required(table,TracksId,0);Tracks result{version,{}};
    for(std::size_t track=0;track<TrackCount;++track){
        std::array<bool,std::size_t(MissingDescriptor)+1> seen{};
        for(std::size_t quarter=0;quarter<MapEntriesPerTrack;++quarter){
            checkCancellation(token);const auto descriptor=map[track*MapEntriesPerTrack+quarter];if(descriptor==MissingDescriptor||seen[descriptor])continue;seen[descriptor]=true;
            result.tracks.push_back({std::int32_t(track),descriptor,trackBits(data,tracks,version,std::int32_t(track),descriptor)});
        }
    }
    return result;
}
std::shared_ptr<const MediaFileSystems::SectorImage> read(MediaFileSystems::Bytes data,std::stop_token token){
    auto parsed=readTracks(data,token);std::vector<Apple::DecodedTrack> standard,rwts;
    for(std::int32_t track=0;track<std::int32_t(Constants::TrackCount);++track){
        auto standardScore=Apple::TrackSelectionConstants::InitialScore,rwtsScore=Apple::TrackSelectionConstants::InitialScore;
        std::optional<Apple::DecodedTrack> bestStandard,bestRwts;
        for(const auto& item:parsed.tracks){if(item.track!=track||item.bits.empty())continue;auto decoded=Apple::decodeTrack(item.bits,track,token);
            if(decoded.standardScore>standardScore){bestStandard=Apple::DecodedTrack{track,std::move(decoded.standard)};standardScore=decoded.standardScore;}
            if(decoded.rwts18Score>rwtsScore){bestRwts=Apple::DecodedTrack{track,std::move(decoded.rwts18)};rwtsScore=decoded.rwts18Score;}
        }
        if(bestStandard)standard.push_back(std::move(*bestStandard));if(bestRwts)rwts.push_back(std::move(*bestRwts));
    }
    const auto credible=std::count_if(rwts.begin(),rwts.end(),[](const auto& track){return !track.sectors.empty();});
    return credible>=Apple::TrackSelectionConstants::MinimumCredibleRwts18TrackCount?Apple::buildRwts18(rwts):Apple::buildStandard(standard);
}
}
