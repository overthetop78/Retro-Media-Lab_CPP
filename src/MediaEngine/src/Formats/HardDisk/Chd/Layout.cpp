#include "Container.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <algorithm>
#include <charconv>
#include <unordered_set>
namespace RetroMediaLab::MediaEngine::ChdHardDisk {
namespace {
HardDiskGeometry geometryFromText(std::span<const std::uint8_t> bytes){
    using namespace Constants;while(!bytes.empty()&&!bytes.back())bytes=bytes.first(bytes.size()-1);std::string_view text(reinterpret_cast<const char*>(bytes.data()),bytes.size());std::array<std::uint64_t,GeometryFields> values{};
    for(int i=0;i<GeometryFields;++i){const auto prefix=GeometryPrefixes[i];if(!text.starts_with(prefix))throw MediaError(DiagnosticCode::ChdHardDiskGeometry);text.remove_prefix(prefix.size());const auto end=i+1<GeometryFields?text.find(GeometrySeparator):text.size();
        if(end==std::string_view::npos||!end)throw MediaError(DiagnosticCode::ChdHardDiskGeometry);const auto result=std::from_chars(text.data(),text.data()+end,values[i]);if(result.ec!=std::errc()||result.ptr!=text.data()+end||!values[i]||values[i]>(i==0?MaximumSigned:MaximumBuffer))throw MediaError(DiagnosticCode::ChdHardDiskGeometry);
        text.remove_prefix(end);if(i+1<GeometryFields)text.remove_prefix(1);
    }
    return HardDiskGeometry(std::int64_t(values[0]),std::int32_t(values[GeometryHeads]),std::int32_t(values[GeometrySectors]),std::int32_t(values[GeometryBytes]));
}
HardDiskGeometry readGeometry(const RecognitionContext& context,std::uint64_t offset,std::stop_token token){
    using namespace Constants;if(!offset)throw MediaError(DiagnosticCode::ChdMetadata);std::unordered_set<std::uint64_t> visited;
    while(offset){checkCancellation(token);if(!visited.insert(offset).second)throw MediaError(DiagnosticCode::ChdMetadata);
        if(offset<HeaderBytes||offset>std::uint64_t(context.length())||MetadataHeaderBytes>std::uint64_t(context.length())-offset)throw MediaError(DiagnosticCode::ChdMetadata);
        const auto header=context.readAsync(std::int64_t(offset),MetadataHeaderBytes,token).get();const auto tag=qFromBigEndian<std::uint32_t>(header.data()),length=qFromBigEndian<std::uint32_t>(header.data()+MetadataLengthOffset)&MetadataLengthMask;const auto next=qFromBigEndian<std::uint64_t>(header.data()+MetadataNextOffset),data=offset+MetadataHeaderBytes;
        if(next>MaximumSigned||!length||length>std::uint64_t(context.length())-data)throw MediaError(DiagnosticCode::ChdMetadata);
        if(tag==MetadataTag)return geometryFromText(context.readAsync(std::int64_t(data),std::int32_t(length),token).get());offset=next;
    }
    throw MediaError(DiagnosticCode::ChdMetadata);
}
}
Layout readLayout(const RecognitionContext& context,std::stop_token token){
    using namespace Constants;checkCancellation(token);if(context.length()<HeaderBytes)throw MediaError(DiagnosticCode::ChdHeader);const auto header=context.readAsync(0,HeaderBytes,token).get();
    const auto u32=[&](int o){return qFromBigEndian<std::uint32_t>(header.data()+o);};const auto u64=[&](int o){return qFromBigEndian<std::uint64_t>(header.data()+o);};
    if(!std::equal(Signature.begin(),Signature.end(),header.begin()))throw MediaError(DiagnosticCode::ChdHeader);
    if(u32(HeaderSizeOffset)!=HeaderBytes||u32(VersionOffset)!=Version5)throw MediaError(DiagnosticCode::ChdHardDiskProfile);
    for(int i=0;i<CodecCount;++i)if(u32(CodecOffset+i*WordBytes))throw MediaError(DiagnosticCode::ChdHardDiskProfile);
    if(std::any_of(header.begin()+ParentOffset,header.begin()+ParentOffset+ParentBytes,[](auto v){return v!=0;}))throw MediaError(DiagnosticCode::ChdParent);
    const auto logical=u64(LogicalOffset),mapOffset=u64(MapOffset),metadata=u64(MetadataOffset);const auto hunk=u32(HunkOffset),unit=u32(UnitOffset);
    if(!logical||logical>MaximumSigned||mapOffset>MaximumSigned||metadata>MaximumSigned||!hunk||hunk>MaximumBuffer||!unit||unit>MaximumBuffer||logical%unit||hunk%unit)throw MediaError(DiagnosticCode::ChdDimensions);
    const auto count=logical/hunk+(logical%hunk!=0);if(count>MaximumBuffer/UncompressedMapBytes)throw MediaError(DiagnosticCode::ChdMap);const auto mapLength=count*UncompressedMapBytes;
    if(mapOffset<HeaderBytes||mapLength>MaximumBuffer||mapOffset>std::uint64_t(context.length())||mapLength>std::uint64_t(context.length())-mapOffset)throw MediaError(DiagnosticCode::ChdMap);
    auto map=context.readAsync(std::int64_t(mapOffset),std::int32_t(mapLength),token).get();auto geometry=readGeometry(context,metadata,token);
    if(std::uint32_t(geometry.sectorSize())!=unit||std::uint64_t(geometry.capacity())!=logical)throw MediaError(DiagnosticCode::ChdHardDiskGeometry);
    return {std::int64_t(logical),hunk,unit,count,std::move(map),std::move(geometry)};
}
}
