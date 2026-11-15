#include "Internal.h"
#include "Constants.h"
#include "../BbcDfs/BbcDfsImages.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
namespace RetroMediaLab::MediaEngine::TeleDisk {
Header createHeader(const MediaFileSystems::SectorImage& image,std::stop_token token){
    using namespace Constants;namespace F=MediaFileSystems::Formats;const auto& g=image.geometry();std::uint8_t rate=DefaultDataRate;
    if(Primitives::ordinalIdentifierEqualsIgnoreCase(g.formatId,F::UcsdIbmMfm)){
        bool first=true,consistent=true;std::optional<std::uint8_t> observed;
        image.visitAvailableBlocks([&](auto index){MediaFileSystems::checkCancellation(token);const auto block=MediaFileSystems::requireSectorBlock(image,index,token);if(first){observed=block->formatCode;first=false;}consistent=consistent&&observed&&block->formatCode==observed&&block->recordedSizeCode.has_value()&&block->diagnosticCode.has_value();return consistent;},token);
        if(!first&&consistent&&observed)rate=*observed;
    }
    if(BbcDfs::findGeometry(g.formatId)||Primitives::ordinalIdentifierEqualsIgnoreCase(g.formatId,F::AcornAtomDos))rate=FmDataRate;
    else if(Primitives::ordinalIdentifierEqualsIgnoreCase(g.formatId,F::AcornAdfs1600))rate=HighDensityDataRate;
    const auto drive=g.cylinders<=LowCylinderLimit&&g.heads<=LowHeadLimit?Drive525Double:image.capacity()<=DoubleDensityCapacity?Drive35Double:image.capacity()<=HighDensity525Capacity?Drive525High:Drive35High;
    return {0,0,DefaultVersion,rate,drive,0,0,checkedByte(g.heads)};
}
}
