#include "SectorPolicies.h"
#include "../Reconstruction/Sector/Policies.h"
#include "../Formats/FormatIds.h"
#include "../Codecs/Amiga/Constants.h"
#include "../Codecs/Iso/FmConstants.h"
#include "../Codecs/Iso/MfmConstants.h"
namespace RetroMediaLab::MediaEngine::Composition {
std::vector<std::shared_ptr<const Reconstruction::Sector::Policy>> defaultSectorPolicies() {
    using namespace Reconstruction::Sector;
    return {
        std::make_shared<ApplePolicy>(),
        std::make_shared<CommodorePolicy>(),
        std::make_shared<Rx02Policy>(),
        std::make_shared<RarePolicy>(),
        std::make_shared<AtariPolicy>(),
        std::make_shared<JfdPolicy>(),
        prefixPolicy(std::string(AmigaMfm::Constants::Id), {std::string(Formats::AmigaPrefix)}),
        prefixPolicy(std::string(IsoFm::Constants::Id), {std::string(Formats::AcornDfsPrefix)}),
        prefixPolicy(std::string(IsoMfm::Constants::Id), {std::string(Formats::AcornAdfsPrefix)}),
        prefixPolicy(std::string(IsoMfm::Constants::Id), {std::string(Formats::IbmPrefix), std::string(Formats::AmstradPrefix),
            std::string(Formats::MsxPrefix), std::string(Formats::UcsdPrefix), std::string(Formats::EpsonQx10Prefix)}),
        exactPolicy(std::string(IsoMfm::Constants::Id), {std::string(Formats::Imd), std::string(Formats::Td0), std::string(Formats::AcornDosPlus800)})
    };
}
}

