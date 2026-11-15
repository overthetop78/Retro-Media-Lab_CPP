#include "Internal.h"
#include <bit>
namespace RetroMediaLab::MediaFileSystems::MacMfs::Internal {
using namespace Constants;
Blocks readBlocks(const SectorImage& image,std::int32_t first,std::int32_t count,BlockContext context,std::vector<Diagnostic>& warnings,std::stop_token token){
    if(count<0||std::uint64_t(count)>MaximumStreamSize/SectorSize)throw MediaError(DiagnosticCode::ReadOutOfBounds);
    Blocks result;result.bytes.resize(std::size_t(count)*SectorSize);result.present.resize(count,false);
    for(std::int32_t index=0;index<count;++index){checkCancellation(token);const auto logical=std::bit_cast<std::int32_t>(std::uint32_t(first)+std::uint32_t(index));const auto block=logical>=0?image.block(std::uint64_t(logical),token):nullptr;
        if(!block||block->data.size()!=SectorSize){warnings.push_back({context==BlockContext::Volume?DiagnosticCode::MfsVolumeBlockMissing:DiagnosticCode::MfsDirectoryBlockMissing,{std::to_string(logical)}});continue;}
        std::copy(block->data.begin(),block->data.end(),result.bytes.begin()+std::size_t(index)*SectorSize);result.present[index]=true;
    }
    result.valid=std::all_of(result.present.begin(),result.present.end(),[](bool value){return value;});return result;
}
Fork readFork(const SectorImage& image,const AllocationMap& map,std::int32_t start,std::uint32_t allocationSize,std::int32_t first,std::uint32_t length,std::string_view file,ForkKind fork,std::vector<Diagnostic>& warnings,std::stop_token token){
    Fork result;if(length==0){result.valid=first==0;return result;}
    if(first==0){warnings.push_back({fork==ForkKind::Data?DiagnosticCode::MfsDataMetadata:DiagnosticCode::MfsResourceMetadata,{std::string(file),std::to_string(length),std::to_string(first)}});return result;}
    if(allocationSize==0||allocationSize%SectorSize)throw MediaError(DiagnosticCode::MacAllocationSize,{std::string(System),std::to_string(allocationSize),std::to_string(SectorSize)});
    const auto blocksPerAllocation=allocationSize/SectorSize;
    // Unsigned addition intentionally retains the C# UInt32 arithmetic before division.
    const auto required=std::int32_t(std::uint32_t(length+allocationSize-1)/allocationSize);const auto chain=map.traverse(first,required);result.clusters=chain.clusters;
    if(!chain.valid){
        const auto code=fork==ForkKind::Data
            ? (chain.cycle?DiagnosticCode::MfsDataChainCycle:chain.outOfRange?DiagnosticCode::MfsDataChainOutside:DiagnosticCode::MfsDataChainPremature)
            : (chain.cycle?DiagnosticCode::MfsResourceChainCycle:chain.outOfRange?DiagnosticCode::MfsResourceChainOutside:DiagnosticCode::MfsResourceChainPremature);
        warnings.push_back({code,{std::string(file)}});
    }
    for(auto cluster:chain.clusters){const auto firstBlock=std::uint32_t(start)+std::uint32_t(cluster-FirstCluster)*blocksPerAllocation;
        for(std::uint32_t index=0;index<blocksPerAllocation&&result.content.size()<length;++index){checkCancellation(token);const auto logical=std::bit_cast<std::int32_t>(firstBlock+index);const auto block=logical>=0?image.block(std::uint64_t(logical),token):nullptr;
            if(result.content.size()>MaximumStreamSize-SectorSize)throw MediaError(DiagnosticCode::ReadOutOfBounds);
            if(!block||block->data.size()!=SectorSize){result.missing.push_back(logical);warnings.push_back({fork==ForkKind::Data?DiagnosticCode::MfsDataBlockMissing:DiagnosticCode::MfsResourceBlockMissing,{std::string(file),std::to_string(logical)}});result.content.resize(result.content.size()+SectorSize,0);}
            else result.content.insert(result.content.end(),block->data.begin(),block->data.end());
        }
    }
    if(result.content.size()<length)warnings.push_back({fork==ForkKind::Data?DiagnosticCode::MfsDataIncomplete:DiagnosticCode::MfsResourceIncomplete,{std::string(file),std::to_string(result.content.size()),std::to_string(length)}});
    result.content.resize(std::min({result.content.size(),std::size_t(length),MaximumStreamSize}));result.valid=chain.valid&&result.missing.empty()&&result.content.size()==length;return result;
}
}
