#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::Cpm::Internal {
namespace {
bool within(std::int32_t allocation,std::int64_t total){return allocation>0&&allocation<total;}
std::int32_t offset(const Layout& layout,std::int32_t allocation){const auto value=std::int64_t(layout.allocationOrigin)+std::int64_t(allocation)*layout.allocationSize;if(value>std::numeric_limits<std::int32_t>::max()||value<std::numeric_limits<std::int32_t>::min())throw MediaError(DiagnosticCode::ReadOutOfBounds,{},ErrorKind::Overflow);return std::int32_t(value);}
void append(std::vector<std::uint8_t>& target,Bytes data){if(data.size()>Constants::MaximumBuffer-target.size())throw MediaError(DiagnosticCode::ReadOutOfBounds);target.insert(target.end(),data.begin(),data.end());}
void zeros(std::vector<std::uint8_t>& target,std::int32_t count){if(std::size_t(count)>Constants::MaximumBuffer-target.size())throw MediaError(DiagnosticCode::ReadOutOfBounds);target.resize(target.size()+count,0);}
}
File reconstruct(const LogicalImage& image,const Layout& layout,const Group& group,std::vector<Diagnostic>& warnings,std::stop_token token){
    const auto total=std::max(std::int64_t(0),(std::int64_t(image.bytes.size())-layout.allocationOrigin)/layout.allocationSize);std::int64_t referenced=0,validCount=0;
    for(const auto& extent:group.extents)for(const auto allocation:extent.allocations){checkCancellation(token);if(allocation==0)continue;++referenced;if(within(allocation,total)&&image.available(offset(layout,allocation),layout.allocationSize))++validCount;}
    File result;if(referenced!=0&&validCount*Constants::MajorityMultiplier<referenced){result.valid=false;result.rejected=true;return result;}
    std::vector<const Extent*> ordered;for(const auto& extent:group.extents)ordered.push_back(&extent);std::stable_sort(ordered.begin(),ordered.end(),[](const auto* a,const auto* b){return a->number<b->number;});
    for(const auto* extent:ordered){
        checkCancellation(token);std::vector<std::uint8_t> bytes;
        for(const auto allocation:extent->allocations){
            checkCancellation(token);if(allocation==0)continue;const auto position=offset(layout,allocation);
            if(!within(allocation,total)){warnings.push_back({DiagnosticCode::CpmAllocationOutside,{group.name,std::to_string(allocation),std::to_string(position),std::to_string(image.bytes.size())}});result.valid=false;zeros(bytes,layout.allocationSize);continue;}
            if(!image.available(position,layout.allocationSize)){
                const auto first=std::int64_t(position)/image.blockSize;const auto last=(std::int64_t(position)+layout.allocationSize-1)/image.blockSize;bool missing=false;for(auto block=std::int64_t(first);block<=last;++block)if(image.missing.contains(std::int32_t(block))){missing=true;break;}
                warnings.push_back({missing?DiagnosticCode::CpmMissingLogicalBlock:DiagnosticCode::CpmTruncatedLogicalBlock,{group.name,std::to_string(allocation)}});result.valid=false;zeros(bytes,layout.allocationSize);continue;
            }
            if(!result.used.insert(allocation).second)warnings.push_back({DiagnosticCode::CpmDuplicateAllocation,{group.name,std::to_string(allocation)}});
            append(bytes,Bytes(image.bytes).subspan(position,layout.allocationSize));
        }
        const auto used=std::min(bytes.size(),std::size_t(extent->recordCount)*Constants::RecordSize);append(result.content,Bytes(bytes).first(used));
    }
    return result;
}
FileSystemVolume volume(const SectorImage& source,const LogicalImage& logical,const Layout& layout,const Directory& directory,std::string_view id,std::stop_token token){
    FileSystemVolume result;if(!logical.missing.empty())result.warnings.push_back({DiagnosticCode::CpmImageMissing,{std::to_string(logical.missing.size())}});if(!logical.truncated.empty())result.warnings.push_back({DiagnosticCode::CpmImageTruncated,{std::to_string(logical.truncated.size())}});
    std::set<std::int32_t> used;for(const auto& group:groups(directory.extents)){checkCancellation(token);auto file=reconstruct(logical,layout,group,result.warnings,token);if(file.rejected)continue;used.insert(file.used.begin(),file.used.end());FileSystemEntry entry;entry.name=group.name;entry.kind=EntryKind::File;entry.size=file.content.size();entry.comment=std::string(Constants::UserPrefix)+std::to_string(group.user);entry.rawAttributes=group.user;entry.storageReference=Constants::NoStorageReference;entry.metadataValid=file.valid;entry.content=std::move(file.content);entry.commentResourceKey=Constants::UserAreaKey;entry.commentTranslationDomain=Constants::TranslationDomain;entry.commentArguments={std::to_string(group.user)};result.entries.push_back(std::move(entry));}
    const auto total=std::max(std::int64_t(0),(std::int64_t(logical.bytes.size())-layout.allocationOrigin)/layout.allocationSize);const auto free=std::max(std::int64_t(0),total-std::int64_t(used.size())-layout.directoryBlocks);
    result.name=directory.volumeName;result.fileSystemId=id;result.fileSystemDisplayName=id;result.capacity=source.capacity();result.freeBytes=std::uint64_t(free)*layout.allocationSize;std::stable_sort(result.entries.begin(),result.entries.end(),[](const auto& a,const auto& b){return lessName(a.name,b.name);});return result;
}
}
