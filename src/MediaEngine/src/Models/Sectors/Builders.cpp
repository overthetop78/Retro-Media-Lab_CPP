#include "Builders.h"
#include "SectorConstants.h"
#include "../../Formats/Apple/Geometry.h"
#include "../../Formats/Raw/Constants.h"
#include "../../Sources/RandomAccessSources.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaEngine {
namespace M=MediaFileSystems;
namespace {
[[noreturn]] void invalid(std::string_view field,std::string value,M::ErrorKind kind=M::ErrorKind::InvalidData){throw M::MediaError(M::DiagnosticCode::SectorInvalidGeometry,{std::string(field),std::move(value)},kind);}
std::uint64_t product(std::uint64_t a,std::uint64_t b,std::string_view field){if(b&&a>std::uint64_t(std::numeric_limits<std::int32_t>::max())/b)invalid(field,std::string(SectorConstants::Overflow),M::ErrorKind::Overflow);return a*b;}
std::uint64_t capacity(const M::SectorGeometry& g){
    for(const auto& [value,field]:{std::pair{g.blockSize,SectorConstants::BlockSizeField},std::pair{g.cylinders,SectorConstants::CylindersField},std::pair{g.heads,SectorConstants::HeadsField},std::pair{g.sectorsPerTrack,SectorConstants::TrackSectorsField}})if(value==0)invalid(field,std::to_string(value));
    const auto count=product(product(g.cylinders,g.heads,SectorConstants::CountField),g.sectorsPerTrack,SectorConstants::CountField);return product(count,g.blockSize,SectorConstants::CapacityField);
}
void exact(std::string_view id,std::uint64_t observed,std::uint64_t expected){if(observed!=expected)throw MediaError(DiagnosticCode::SectorBuilderLength,{std::string(id),std::to_string(observed),std::to_string(expected)});}
}
std::shared_ptr<const M::SectorImage> linearSectorImage(std::shared_ptr<const M::RandomAccessData> source,M::SectorGeometry g,SectorNumbering numbering,std::stop_token token){
    checkCancellation(token);if(!source)throw M::MediaError(M::DiagnosticCode::InvalidSource);exact(g.formatId,source->length(),capacity(g));
    SourceSectorMetadata metadata;metadata.firstSectorNumber=numbering==SectorNumbering::OneBased?1:0;metadata.trustCompletePayload=true;return sourceSectorImage(std::move(source),std::move(g),0,{},metadata);
}
std::shared_ptr<const M::SectorImage> regularSectorImage(M::Bytes bytes,M::SectorGeometry g,std::int32_t first,std::int32_t trailing,std::stop_token token){
    checkCancellation(token);const auto length=capacity(g);if(trailing<0)invalid(SectorConstants::CapacityField,std::to_string(trailing));exact(g.formatId,bytes.size(),length+std::uint64_t(trailing));
    return linearSectorImage(memorySource(bytes.first(static_cast<std::size_t>(length))),std::move(g),first==0?SectorNumbering::ZeroBased:SectorNumbering::OneBased,token);
}
std::shared_ptr<const M::SectorImage> ibmRawSectorImage(std::shared_ptr<const M::RandomAccessData> source,M::SectorGeometry g,std::stop_token token){g.blockSize=Raw::Constants::SectorSize;return linearSectorImage(std::move(source),std::move(g),SectorNumbering::OneBased,token);}
std::shared_ptr<const M::SectorImage> macintoshSectorImage(M::Bytes bytes,std::string format,std::int32_t heads,std::stop_token token){
    checkCancellation(token);namespace G=Apple::Geometry::MacintoshGcr;const auto g=G::forHeads(heads);exact(format,bytes.size(),g.capacity);
    SourceSectorMetadata metadata;metadata.trustCompletePayload=true;metadata.addressOf=[heads](auto logical){return G::address(static_cast<std::int32_t>(logical),heads);};
    M::SectorImageOptions options;options.capacity=g.capacity;options.logicalBlockCount=g.blockCount;
    return sourceSectorImage(memorySource(bytes),{std::move(format),G::BlockSize,G::CylinderCount,static_cast<std::uint32_t>(heads),G::MaximumSectorsPerTrack},0,options,metadata);
}
DenseSectorImage denseSectorImage(std::span<const M::SectorBlock> blocks,std::int32_t count,std::int32_t size,std::stop_token token){
    checkCancellation(token);if(count<0)invalid(SectorConstants::CountField,std::to_string(count));if(size<0)invalid(SectorConstants::BlockSizeField,std::to_string(size));
    DenseSectorImage result;result.data.resize(static_cast<std::size_t>(product(static_cast<std::uint32_t>(count),static_cast<std::uint32_t>(size),SectorConstants::CapacityField)));std::vector<bool> present(static_cast<std::size_t>(count));
    for(const auto& block:blocks){
        checkCancellation(token);if(block.logicalBlock>=static_cast<std::uint64_t>(count))throw M::MediaError(M::DiagnosticCode::SectorBlockOutside,{std::to_string(block.logicalBlock),std::to_string(count)});
        const auto offset=static_cast<std::size_t>(block.logicalBlock)*static_cast<std::size_t>(size);if(block.data.size()>result.data.size()-offset)throw M::MediaError(M::DiagnosticCode::ReadOutOfBounds);
        std::copy(block.data.begin(),block.data.end(),result.data.begin()+static_cast<std::ptrdiff_t>(offset));present[static_cast<std::size_t>(block.logicalBlock)]=true;
    }
    for(std::int32_t index=0;index<count;++index){checkCancellation(token);if(!present[static_cast<std::size_t>(index)])result.missingBlocks.push_back(index);}return result;
}
}
