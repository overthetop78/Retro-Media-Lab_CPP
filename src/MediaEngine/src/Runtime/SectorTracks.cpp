#include "SectorTracks.h"
#include "../Models/Sectors/SectorImages.h"
#include "../Models/Sectors/SizeCodes.h"
#include "../Models/Sectors/SectorConstants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <limits>
#include <set>
#include <tuple>
namespace RetroMediaLab::MediaEngine::RuntimeDetails {
namespace M=MediaFileSystems;
namespace {
[[noreturn]] void invalid(){throw MediaError(DiagnosticCode::RuntimeDiskSector);}
bool onTrack(const M::SectorBlock& b,std::int32_t cylinder,std::int32_t head){return b.address.cylinder==cylinder&&b.address.head==head;}
M::SectorBlock create(const MediaDiskSector& field,std::int32_t cylinder,std::int32_t head,std::uint64_t logical,std::size_t order){
    if(order>static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()))invalid();
    M::SectorBlock b;b.logicalBlock=logical;b.address={cylinder,head,field.number};b.data=field.data;b.integrityValid=field.integrityValid;
    b.recordedAddress=M::SectorAddress{field.cylinder,field.head,field.number};b.recordedSizeCode=field.recordedSizeCode;b.recordedTrackOrder=static_cast<std::int32_t>(order);
    b.headerIntegrityValid=field.headerIntegrityValid;b.dataIntegrityValid=field.dataIntegrityValid;b.deletedDataMark=field.deletedDataMark;b.diagnosticCode=field.diagnosticCode;
    b.recordedErrorCheckCode=field.recordedErrorCheckCode;b.recordedDataPrefix=field.recordedDataPrefix;return b;
}
std::vector<M::SectorBlock> available(const M::SectorImage& image){std::vector<M::SectorBlock> result;image.visitAvailableBlocks([&](auto i){if(auto b=image.block(i))result.push_back(*b);return true;});return result;}
std::uint64_t totalBytes(std::span<const M::SectorBlock> blocks){std::uint64_t result=0;for(const auto& b:blocks){if(b.data.size()>SectorConstants::CapacityLimit-result)invalid();result+=b.data.size();}return result;}
}
void validateTrack(const M::SectorImage& image,std::int32_t cylinder,std::int32_t head,std::span<const MediaDiskSector> fields){
    if(cylinder<0||head<0||static_cast<std::uint32_t>(cylinder)>=image.geometry().cylinders||static_cast<std::uint32_t>(head)>=image.geometry().heads||fields.empty())invalid();
    for(const auto& f:fields)if(f.data.empty()||f.cylinder<0||f.head<0||f.number<0)invalid();
}
std::shared_ptr<const M::SectorImage> replaceTrack(const M::SectorImage& image,std::int32_t cylinder,std::int32_t head,std::span<const MediaDiskSector> fields,bool retainGeometry,std::optional<std::span<const M::SectorBlock>> previousTrack){
    validateTrack(image,cylinder,head,fields);auto original=available(image);std::vector<M::SectorBlock> previous;
    if(previousTrack)previous.assign(previousTrack->begin(),previousTrack->end());else for(const auto& b:original)if(onTrack(b,cylinder,head))previous.push_back(b);
    std::vector<M::SectorBlock> result;for(const auto& b:original)if(!onTrack(b,cylinder,head))result.push_back(b);
    if(retainGeometry){
        if(fields.size()!=previous.size())invalid();std::set<std::int64_t> numbers,oldNumbers;
        for(const auto& b:previous)if(!oldNumbers.insert(b.address.number).second)invalid();
        for(std::size_t i=0;i<fields.size();++i){const auto& f=fields[i];if(!numbers.insert(f.number).second)invalid();
            const auto found=std::find_if(previous.begin(),previous.end(),[&](const auto& b){return b.address.number==f.number;});if(found==previous.end())invalid();
            const auto recorded=found->recordedAddress.value_or(found->address);
            if(f.cylinder!=recorded.cylinder||f.head!=recorded.head||f.data.size()!=found->data.size()||f.deletedDataMark==true||f.headerIntegrityValid==false||f.dataIntegrityValid==false)invalid();
            if(f.recordedSizeCode&&( *f.recordedSizeCode>SectorConstants::MaximumIsoSizeCode||sectorSizeBytes(*f.recordedSizeCode)!=f.data.size()))invalid();
            result.push_back(create(f,cylinder,head,found->logicalBlock,i));
        }
        M::SectorImageOptions options;options.allowVariableBlockSize=image.allowsVariableBlockSize();options.capacity=image.capacity();options.logicalBlockCount=image.blockCount();
        return sparseSectorImage(image.geometry(),std::move(result),options);
    }
    if(original.size()!=image.blockCount())invalid();
    for(std::size_t i=0;i<fields.size();++i)result.push_back(create(fields[i],cylinder,head,i,i));
    std::stable_sort(result.begin(),result.end(),[](const auto& a,const auto& b){return std::tie(a.address.cylinder,a.address.head,a.address.number)<std::tie(b.address.cylinder,b.address.head,b.address.number);});
    std::map<std::pair<std::int64_t,std::int64_t>,std::uint64_t> counts;std::uint64_t largest=0;
    for(std::size_t i=0;i<result.size();++i){result[i].logicalBlock=i;largest=std::max(largest,++counts[{result[i].address.cylinder,result[i].address.head}]);}
    if(largest>std::numeric_limits<std::uint32_t>::max())invalid();auto geometry=image.geometry();geometry.sectorsPerTrack=static_cast<std::uint32_t>(largest);
    M::SectorImageOptions options;options.capacity=totalBytes(result);options.logicalBlockCount=result.size();options.allowVariableBlockSize=std::any_of(result.begin(),result.end(),[&](const auto& b){return b.data.size()!=geometry.blockSize;});
    return sparseSectorImage(std::move(geometry),std::move(result),options);
}
std::shared_ptr<const M::SectorImage> projectTrack(const M::SectorImage& image,std::int32_t cylinder,std::int32_t head,std::span<const MediaDiskSector> fields){
    auto original=available(image);std::vector<M::SectorBlock> previous;std::map<std::uint64_t,M::SectorBlock> values;
    for(auto& b:original){if(onTrack(b,cylinder,head))previous.push_back(std::move(b));else values.emplace(b.logicalBlock,std::move(b));}
    std::stable_sort(previous.begin(),previous.end(),[](const auto& a,const auto& b){
        const auto left=a.recordedTrackOrder?static_cast<std::int64_t>(*a.recordedTrackOrder):static_cast<std::int64_t>(a.logicalBlock);
        const auto right=b.recordedTrackOrder?static_cast<std::int64_t>(*b.recordedTrackOrder):static_cast<std::int64_t>(b.logicalBlock);return left<right;
    });
    auto next=image.blockCount();for(std::size_t i=0;i<fields.size();++i){if(i>=previous.size()&&next==SectorConstants::CapacityLimit)invalid();const auto logical=i<previous.size()?previous[i].logicalBlock:next++;if(!values.emplace(logical,create(fields[i],cylinder,head,logical,i)).second)invalid();}
    std::vector<M::SectorBlock> result;for(auto& [i,b]:values)result.push_back(std::move(b));
    M::SectorImageOptions options;options.allowVariableBlockSize=true;options.logicalBlockCount=next;options.capacity=std::max(image.capacity(),totalBytes(result));
    return sparseSectorImage(image.geometry(),std::move(result),options);
}
}
