#include "Reader.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <charconv>
#include <unordered_map>
#include <unordered_set>
namespace RetroMediaLab::MediaEngine::ChdOptical {
namespace {
using namespace Constants;
std::int64_t number(std::string_view value,bool positive){
    std::uint64_t result=0;const auto parsed=std::from_chars(value.data(),value.data()+value.size(),result);if(value.empty()||parsed.ec!=std::errc()||parsed.ptr!=value.data()+value.size()||result>MaximumSigned||(positive&&!result))throw MediaError(DiagnosticCode::ChdOpticalMetadata);return std::int64_t(result);
}
TrackMetadata parse(std::uint32_t tag,std::string_view text){
    if(!text.starts_with(TrackPrefix))throw MediaError(DiagnosticCode::ChdOpticalMetadata);std::unordered_map<std::string_view,std::string_view> values;
    while(!text.empty()){const auto end=text.find(TokenSeparator);const auto token=text.substr(0,end);const auto separator=token.find(ValueSeparator);if(separator!=std::string_view::npos&&!values.emplace(token.substr(0,separator),token.substr(separator+1)).second)throw MediaError(DiagnosticCode::ChdOpticalMetadata);if(end==std::string_view::npos)break;text.remove_prefix(end+1);}
    const auto count=tag==Track2Tag?RequiredKeys.size():std::size_t(SimpleKeys);for(std::size_t i=0;i<count;++i)if(!values.contains(RequiredKeys[i]))throw MediaError(DiagnosticCode::ChdOpticalMetadata);
    TrackMetadata result;const auto track=number(values.at(TrackKey),true);if(std::uint64_t(track)>MaximumBuffer)throw MediaError(DiagnosticCode::ChdOpticalMetadata);result.number=int(track);result.frames=number(values.at(FramesKey),true);
    const auto type=values.at(TypeKey);bool found=false;for(const auto& entry:TrackTypes)if(entry.name==type){result.mode=entry.mode;result.dataSize=entry.size;found=true;break;}if(!found)throw MediaError(DiagnosticCode::ChdOpticalTrackType,{std::string(type)});
    const auto subtype=values.at(SubtypeKey);if(subtype==NoSubchannel)result.subchannelSize=0;else if(subtype==RwSubchannel||subtype==RawSubchannel)result.subchannelSize=Subchannel96;else throw MediaError(DiagnosticCode::ChdOpticalSubchannel,{std::string(subtype)});
    if(tag==Track2Tag){result.pregap=number(values.at(PregapKey),false);result.postgap=number(values.at(PostgapKey),false);const auto pgtype=values.at(PgtypeKey);result.storedPregap=!pgtype.empty()&&pgtype.front()==StoredPregapPrefix;}
    if(result.pregap>=result.frames)throw MediaError(DiagnosticCode::ChdOpticalMetadata);return result;
}
}
void readMetadata(const RecognitionContext& context,std::uint64_t offset,Layout& layout,std::stop_token token){
    using namespace Constants;if(!offset)throw MediaError(DiagnosticCode::ChdOpticalMetadata);std::unordered_set<std::uint64_t> visited;
    while(offset){checkCancellation(token);if(!visited.insert(offset).second||offset<HeaderBytes||offset>std::uint64_t(context.length())||MetadataHeaderBytes>std::uint64_t(context.length())-offset)throw MediaError(DiagnosticCode::ChdOpticalMetadata);
        const auto header=context.readAsync(std::int64_t(offset),MetadataHeaderBytes,token).get();const auto tag=qFromBigEndian<std::uint32_t>(header.data()),length=qFromBigEndian<std::uint32_t>(header.data()+MetadataLengthOffset)&MetadataLengthMask;const auto next=qFromBigEndian<std::uint64_t>(header.data()+MetadataNextOffset),dataOffset=offset+MetadataHeaderBytes;
        if(next>MaximumSigned||!length||length>std::uint64_t(context.length())-dataOffset)throw MediaError(DiagnosticCode::ChdOpticalMetadata);
        if(tag==TrackTag||tag==Track2Tag){auto bytes=context.readAsync(std::int64_t(dataOffset),std::int32_t(length),token).get();while(!bytes.empty()&&!bytes.back())bytes.pop_back();for(auto& value:bytes)if(value>=AsciiLimit)value=std::uint8_t(AsciiReplacement);layout.tracks.push_back(parse(tag,std::string_view(reinterpret_cast<const char*>(bytes.data()),bytes.size())));}else if(tag==DvdTag)layout.dvd=true;offset=next;
    }
    if(layout.tracks.empty()&&!layout.dvd)throw MediaError(DiagnosticCode::ChdOpticalMetadata);
}
}
