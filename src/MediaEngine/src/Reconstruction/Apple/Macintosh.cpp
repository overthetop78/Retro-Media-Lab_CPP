#include "Reconstructors.h"
#include "Constants.h"
#include "../../Codecs/AppleIwm/Constants.h"
#include "../../Formats/Apple/Geometry.h"
#include "../../Formats/Apple/Constants.h"
#include "../../Formats/Apple/Probe.h"
#include "../../Formats/DiskCopy/Constants.h"
#include "../../Formats/FormatIds.h"
#include "../../Models/Sectors/Builders.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaFileSystems/AppleRecognitionConstants.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::AppleScp {
namespace M=MediaFileSystems;namespace G=Apple::Geometry::MacintoshGcr;
std::shared_ptr<const M::SectorImage> macintosh(const Scp::Image& image,const Decoder& decoder,const std::optional<std::string>& requested,const IsoReconstruction::CollectionProgressCallback& progress,std::stop_token token){
    const auto candidates=decoder.collect(image,AppleIwm::Constants::MacId,AppleIwm::Constants::SectorByteCount,progress,token);if(candidates.empty())throw MediaError(DiagnosticCode::ScpNoDecodedSectors,{std::string(Constants::MacSectorName)});
    const auto heads=std::any_of(candidates.entries().begin(),candidates.entries().end(),[](const auto& group){return group.address.head==Apple::Geometry::DoubleHeads-1;})?Apple::Geometry::DoubleHeads:Apple::Geometry::SingleHead;
    std::vector<M::SectorBlock> blocks;
    for(const auto& group:candidates.entries()){
        checkCancellation(token);const auto& a=group.address;if(a.cylinder>=G::CylinderCount||a.head>=heads)continue;const auto sectors=G::sectors(static_cast<std::int32_t>(a.cylinder));if(a.number<0||a.number>=sectors)continue;
        std::int32_t prior=0;for(std::int32_t c=0;c<a.cylinder;++c)prior+=G::sectors(c)*heads;
        blocks.push_back(select(static_cast<std::uint64_t>(prior+a.head*sectors+a.number),a,group.candidates,token));
    }
    if(blocks.empty())throw MediaError(DiagnosticCode::ScpNoUsableSectors,{std::string(Constants::MacSectorName),std::to_string(candidates.entries().size()),std::to_string(blocks.size())});
    const auto count=G::SingleSidedBlockCount*heads;std::string format=requested&&Primitives::asciiIdentifierStartsWithIgnoreCase(*requested,Formats::AppleLisaPrefix)?*requested:std::string(Apple::Constants::MacGcr);
    if(!requested&&std::any_of(blocks.begin(),blocks.end(),[](const auto& b){return b.tag&&b.tag->size()>=Constants::LisaTagMinimum&&(*b.tag)[Constants::LisaTagHigh]==Constants::LisaTagHighValue&&(*b.tag)[Constants::LisaTagLow]==Constants::LisaTagLowValue;}))format=Formats::AppleLisaOffice;
    if(!requested&&blocks.size()==static_cast<std::size_t>(count)){
        const auto payload=denseSectorImage(blocks,count,G::BlockSize,token);if(payload.missingBlocks.empty()&&Apple::looksLikeLisaOffice(payload.data))format=Formats::AppleLisaRaw;
    }
    const auto mdb=std::find_if(blocks.begin(),blocks.end(),[](const auto& block){return block.logicalBlock==DiskCopy::Constants::PrebootSearchBlockIndex;});
    if(mdb!=blocks.end()&&mdb->data.size()>=M::Apple::RecognitionConstants::SignatureBytes){
        const auto end=mdb->data.begin()+static_cast<std::ptrdiff_t>(std::min(DiskCopy::Constants::PrebootSearchLength,mdb->data.size()));
        if(std::search(mdb->data.begin(),end,DiskCopy::Constants::PrebootMarker.begin(),DiskCopy::Constants::PrebootMarker.end())!=end)format=DiskCopy::Constants::AppleLisaMacWorks;
        const auto signature=(std::uint16_t(mdb->data[0])<<AppleIwm::Constants::BitsPerByte)|mdb->data[1];
        if(!Primitives::asciiIdentifierStartsWithIgnoreCase(format,Formats::AppleLisaPrefix))format=signature==M::Apple::RecognitionConstants::MfsSignature?Formats::AppleMacMfs:signature==M::Apple::RecognitionConstants::HfsSignature?Formats::AppleMacHfs:Formats::AppleIIProDos;
    }
    M::SectorImageOptions options;options.capacity=std::uint64_t(count)*G::BlockSize;options.logicalBlockCount=count;
    return sparseSectorImage({std::move(format),G::BlockSize,G::CylinderCount,static_cast<std::uint32_t>(heads),G::MaximumSectorsPerTrack},std::move(blocks),options);
}
}
