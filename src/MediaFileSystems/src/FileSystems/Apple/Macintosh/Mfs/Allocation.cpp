#include "Internal.h"
#include <set>
namespace RetroMediaLab::MediaFileSystems::MacMfs::Internal {
using namespace Constants;
AllocationMap AllocationMap::decode(Bytes packed,std::int32_t count){
    if(count<0||count>MaximumAllocations)throw MediaError(DiagnosticCode::MfsMapTruncated,{std::to_string(count),std::to_string(packed.size()),std::to_string(0)});
    const auto required=(std::size_t(count)*BitsPerAllocation+ByteShift-1)/ByteShift;
    if(packed.size()<required)throw MediaError(DiagnosticCode::MfsMapTruncated,{std::to_string(count),std::to_string(packed.size()),std::to_string(required)});
    AllocationMap result;result.entries.resize(count);
    for(std::int32_t index=0;index<count;++index){
        const auto offset=std::size_t(index/EntriesPerPackedPair)*PackedPairLength;
        const auto value=index%EntriesPerPackedPair==0
            ? (std::uint16_t(packed[offset])<<HalfByteShift)|(packed[offset+1]>>HalfByteShift)
            : (std::uint16_t(packed[offset+1]&LowNibble)<<ByteShift)|packed[offset+EntriesPerPackedPair];
        result.entries[index]=std::uint16_t(value&AllocationMask);
    }
    return result;
}
Chain AllocationMap::traverse(std::int32_t first,std::int32_t required)const{
    Chain result;std::set<std::int32_t> visited;auto current=first;
    while(current>=FirstCluster&&current<EndOfChain&&std::int64_t(result.clusters.size())<required){
        if(!visited.insert(current).second){result.cycle=true;break;}const auto index=current-FirstCluster;
        if(index<0||std::size_t(index)>=entries.size()){result.outOfRange=true;break;}
        result.clusters.push_back(current);current=entries[index];
    }
    result.premature=std::int64_t(result.clusters.size())<required&&!result.cycle&&!result.outOfRange;result.valid=!result.cycle&&!result.outOfRange&&!result.premature;return result;
}
}
