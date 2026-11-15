#include "Internal.h"
#include "../../../Primitives/Binary.h"
#include <algorithm>
namespace RetroMediaLab::MediaFileSystems::AmigaFlatArchive::Internal {
namespace {
std::optional<Descriptor> descriptor(Bytes bytes,std::size_t offset){
    using namespace Constants;
    if(offset>bytes.size()||bytes.size()-offset<EntryLength)return {};
    const auto name=bytes.subspan(offset,NameLength);const auto end=std::find(name.begin(),name.end(),0);
    if(end==name.begin()||end==name.end())return {};
    if(!std::all_of(name.begin(),end,[](auto c){return c>=AsciiFirst&&c<=AsciiLast;})||!std::all_of(end+1,name.end(),[](auto c){return c==0||c==Padding;}))return {};
    return Descriptor{std::string(name.begin(),end),Binary::big<std::uint32_t>(bytes,offset+SizeOffset)};
}
}
std::optional<std::vector<Descriptor>> directory(const SectorImage& image,std::stop_token token){
    using namespace Constants;checkCancellation(token);const auto size=image.geometry().blockSize;
    if(size<EntryLength)return {};
    const auto first=image.block(DirectoryStartBlock,token);if(!first)return {};
    const auto reserved=descriptor(first->data,0);
    if(!reserved||reserved->name!=ReservedName||reserved->length<(DirectoryStartBlock+std::uint64_t(1))*size||reserved->length>image.capacity())return {};
    const auto start=std::uint64_t(DirectoryStartBlock)*size,length=reserved->length-start;
    if(length<EntryLength||length>image.capacity()||length>MaximumRangeLength)return {};
    const auto read=data(image,start,length,token);if(!read.missing.empty()||!read.invalid.empty())return {};
    std::vector<Descriptor> parsed;std::uint64_t payload=0;
    for(std::size_t offset=0;offset+EntryLength<=read.bytes.size();offset+=EntryLength){
        checkCancellation(token);if(read.bytes[offset]==Padding)break;
        const auto item=descriptor(read.bytes,offset);if(!item||item->length>image.capacity()-payload)return {};
        payload+=item->length;parsed.push_back(*item);
    }
    if(parsed.size()<DirectoryStartBlock||parsed.front().name!=ReservedName)return {};
    return parsed;
}
}
