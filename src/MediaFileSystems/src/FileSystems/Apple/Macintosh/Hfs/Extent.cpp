#include "Internal.h"
#include <bit>
namespace RetroMediaLab::MediaFileSystems::MacHfs::Internal {
using namespace Constants;
ExtentResult extents(const SectorImage& image,Bytes descriptors,std::uint16_t start,std::uint32_t size,std::uint32_t length,std::stop_token token){
    checkCancellation(token);if(size==0||size%SectorSize)throw MediaError(DiagnosticCode::MacAllocationSize,{std::string(SystemName),std::to_string(size),std::to_string(SectorSize)});
    ExtentResult result;const auto perAllocation=size/SectorSize;
    for(std::int32_t extent=0;extent<EmbeddedExtentCount&&result.content.size()<length;++extent){
        const auto offset=std::size_t(extent*ExtentDescriptorLength);const auto first=Binary::big<std::uint16_t>(descriptors,offset+ExtentStartOffset),count=Binary::big<std::uint16_t>(descriptors,offset+ExtentCountOffset);
        for(std::uint32_t allocation=0;allocation<count&&result.content.size()<length;++allocation){
            for(std::uint32_t index=0;index<perAllocation&&result.content.size()<length;++index){
                checkCancellation(token);const auto logical=std::bit_cast<std::int32_t>(std::uint32_t(start)+(std::uint32_t(first)+allocation)*perAllocation+index);const auto block=logical>=0?image.block(std::uint64_t(logical),token):nullptr;
                if(result.content.size()>MaximumStreamSize-SectorSize)throw MediaError(DiagnosticCode::ReadOutOfBounds);
                if(!block||block->data.size()!=SectorSize){result.missing.push_back(logical);result.content.resize(result.content.size()+SectorSize,0);}
                else result.content.insert(result.content.end(),block->data.begin(),block->data.end());
            }
        }
    }
    result.remaining=length>result.content.size()?std::uint64_t(length)-result.content.size():0;result.content.resize(std::min(result.content.size(),std::size_t(length)));result.valid=result.missing.empty()&&result.remaining==0;return result;
}
void extentWarnings(std::string_view file,Macintosh::ForkKind fork,std::uint32_t length,const ExtentResult& result,std::vector<Diagnostic>& warnings){
    for(auto block:result.missing)warnings.push_back({fork==Macintosh::ForkKind::Data?DiagnosticCode::MfsDataBlockMissing:DiagnosticCode::MfsResourceBlockMissing,{std::string(file),std::to_string(block)}});
    if(result.remaining)warnings.push_back({fork==Macintosh::ForkKind::Data?DiagnosticCode::MfsDataIncomplete:DiagnosticCode::MfsResourceIncomplete,{std::string(file),std::to_string(std::uint64_t(length)-result.remaining),std::to_string(length)}});
}
}
