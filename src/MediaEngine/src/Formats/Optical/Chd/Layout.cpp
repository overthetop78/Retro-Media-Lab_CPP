#include "Reader.h"
#include "../../Common/Chd/Container.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::ChdOptical {
Layout readLayout(const RecognitionContext& context,std::stop_token token){
    using namespace Constants;checkCancellation(token);if(context.length()<HeaderBytes)throw MediaError(DiagnosticCode::ChdHeader);const auto header=context.readAsync(0,HeaderBytes,token).get();const auto u32=[&](int o){return qFromBigEndian<std::uint32_t>(header.data()+o);};const auto u64=[&](int o){return qFromBigEndian<std::uint64_t>(header.data()+o);};
    if(!std::equal(Signature.begin(),Signature.end(),header.begin()))throw MediaError(DiagnosticCode::ChdHeader);Layout result;
    if(u32(VersionOffset)!=Version5||std::any_of(header.begin()+CodecOffset,header.begin()+LogicalOffset,[](auto v){return v!=0;})){
        Chd::Container container(context.dataSource(),token);readMetadata(context,container.metadataOffset(),result,token);result.version=container.version();result.logicalBytes=std::int64_t(container.logicalBytes());result.hunkBytes=container.hunkBytes();result.unitBytes=result.version==Version5?container.unitBytes():result.dvd?Data2048:CdBytes;result.decoded=true;const auto compression=container.compression();result.compression=compression.empty()?std::string(NoCompression):compression;return result;
    }
    if(u32(HeaderSizeOffset)!=HeaderBytes)throw MediaError(DiagnosticCode::ChdHeader);if(std::any_of(header.begin()+ParentOffset,header.begin()+ParentOffset+ParentBytes,[](auto v){return v!=0;}))throw MediaError(DiagnosticCode::ChdParent);
    const auto logical=u64(LogicalOffset),mapOffset=u64(MapOffset),metadata=u64(MetadataOffset);const auto hunk=u32(HunkOffset),unit=u32(UnitOffset);
    if(!logical||logical>MaximumSigned||mapOffset>MaximumSigned||metadata>MaximumSigned||!hunk||hunk>MaximumBuffer||!unit||unit>MaximumBuffer||logical%unit||hunk%unit)throw MediaError(DiagnosticCode::ChdDimensions);
    const auto count=logical/hunk+(logical%hunk!=0);if(count>MaximumBuffer/UncompressedMapBytes)throw MediaError(DiagnosticCode::ChdMap);const auto mapLength=count*UncompressedMapBytes;
    if(mapOffset<HeaderBytes||mapOffset>std::uint64_t(context.length())||mapLength>std::uint64_t(context.length())-mapOffset)throw MediaError(DiagnosticCode::ChdMap);const auto map=context.readAsync(std::int64_t(mapOffset),std::int32_t(mapLength),token).get();result.storedHunks.resize(std::size_t(count));
    for(std::size_t i=0;i<result.storedHunks.size();++i){if(!(i&PollMask))checkCancellation(token);result.storedHunks[i]=qFromBigEndian<std::uint32_t>(map.data()+i*UncompressedMapBytes);}
    result.logicalBytes=std::int64_t(logical);result.hunkBytes=hunk;result.unitBytes=unit;readMetadata(context,metadata,result,token);return result;
}
}
