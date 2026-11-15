#include "../Codecs/HpMmfm/Constants.h"
#include "../Codecs/DataGeneral/Constants.h"
#include "../Codecs/Micropolis/Constants.h"
#include "../Codecs/Membrain/Constants.h"
#include "../Codecs/Aed6200p/Constants.h"
#include "../Codecs/QdMo5/Constants.h"
#include "../Codecs/Centurion/Constants.h"
#include "../Codecs/Northstar/Constants.h"
#include "../Codecs/Heathkit/Constants.h"
#include "../Codecs/MicralN/Constants.h"
#include "../Codecs/Emu/Constants.h"
#include "../Codecs/Tycom/Constants.h"
#include "../Codecs/Arburg/Constants.h"
#include "../Codecs/Victor9000/Constants.h"
#include "RareProfiles.h"
#include "Constants.h"
#include "TimingConstants.h"
#include "../Formats/FormatIds.h"
#include "../Primitives/Identifiers.h"
#include <array>

namespace RetroMediaLab::MediaEngine::Encoding {
namespace {
struct Entry { std::string_view formatId; RareProfile profile; };
constexpr RareProfile defaults(std::string_view id) { return {id, Constants::DefaultCellTicks, Constants::DefaultIndexTicks}; }
constexpr std::array entries{
    Entry{Formats::HpMmfm, {HpMmfm::Constants::Id, TimingConstants::HighDensityMfmCell, TimingConstants::Rpm300Index}},
    Entry{Formats::DataGeneralFm, {DataGeneral::Constants::Id, TimingConstants::DoubleDensityMfmCell, TimingConstants::Rpm300Index}},
    Entry{Formats::MicropolisMfm, {Micropolis::Constants::Id, TimingConstants::DoubleDensityMfmCell, TimingConstants::Rpm300Index}},
    Entry{Formats::MembrainMfm, defaults(Membrain::Constants::Id)},
    Entry{Formats::Aed6200pMfm, {Aed6200p::Constants::Id, TimingConstants::HighDensityMfmCell, TimingConstants::Rpm360Index}},
    Entry{Formats::QdMo5Mfm, defaults(QdMo5::Constants::Id)},
    Entry{Formats::CenturionMfm, defaults(Centurion::Constants::Id)},
    Entry{Formats::NorthstarMfm, defaults(Northstar::Constants::Id)},
    Entry{Formats::HeathkitFm, defaults(Heathkit::Constants::Id)},
    Entry{Formats::MicralNFm, defaults(MicralN::Constants::Id)},
    Entry{Formats::EmuFm, defaults(EmuFm::Constants::Id)},
    Entry{Formats::TycomFm, defaults(Tycom::Constants::Id)},
    Entry{Formats::Arburg, defaults(Arburg::Constants::Id)},
    Entry{Formats::Victor9kGcr, defaults(Victor9000::Constants::Id)}
};
}
std::optional<RareProfile> rareProfile(std::string_view id) {
    for (const auto& entry : entries)
        if (Primitives::asciiIdentifierEqualsIgnoreCase(id, entry.formatId)) return entry.profile;
    return std::nullopt;
}
}

