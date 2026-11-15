#include "Internal.h"
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaFileSystems::Adfs::Internal {
std::optional<std::vector<std::uint8_t>> file(Context& context,std::int32_t address,std::uint32_t length,const std::string& name,FileSystemEntry& entry,std::stop_token token){
    checkCancellation(token);if(length==0)return std::vector<std::uint8_t>{};
    const auto invalid=[&](std::uint64_t offset){Diagnostic warning{DiagnosticCode::AdfsInvalidRange,{name,std::to_string(address),std::to_string(offset),std::to_string(length)}};context.warnings.push_back(warning);entry.diagnostics.push_back(std::move(warning));entry.metadataValid=false;};
    if(length>std::uint32_t(std::numeric_limits<std::int32_t>::max())||length>context.image.capacity()||address<=0||!context.resolver->resolve(address,0,token)){invalid(0);return {};}
    std::vector<std::uint8_t> output(length);std::size_t copied=0;const auto size=context.image.geometry().blockSize;
    while(copied<output.size()){
        checkCancellation(token);const auto position=context.resolver->resolve(address,std::int64_t(copied),token);
        if(!position){invalid(copied);return {};}
        const auto number=*position/size,intra=*position%size;const auto block=context.image.block(number,token);
        if(!block||intra>=block->data.size()){
            Diagnostic warning{DiagnosticCode::AdfsMissingBlock,{name,std::to_string(address),std::to_string(copied),std::to_string(number)}};
            context.warnings.push_back(warning);entry.diagnostics.push_back(std::move(warning));entry.metadataValid=false;return {};
        }
        if(!validIntegrity(*block)){Diagnostic warning{DiagnosticCode::AdfsIntegrity,{std::to_string(number)}};context.warnings.push_back(warning);entry.diagnostics.push_back(std::move(warning));entry.metadataValid=false;entry.dataValid=false;}
        const auto count=std::min(block->data.size()-std::size_t(intra),output.size()-copied);
        std::copy_n(block->data.begin()+std::size_t(intra),count,output.begin()+copied);copied+=count;
    }
    return output;
}
}
