#pragma once
#include "../../Reconstruction/Scp/Candidates.h"
#include "../../Codecs/Amiga/Constants.h"
#include "../../Codecs/Iso/MfmConstants.h"
#include "../../Codecs/Iso/FmConstants.h"
#include "../../Codecs/Commodore/Commodore1541Constants.h"
#include "../../Codecs/AppleII/Constants.h"
#include "../../Codecs/AppleRwts18/Constants.h"
#include "../../Codecs/AppleIwm/Constants.h"
#include "../../Codecs/DecRx02/Constants.h"
#include <array>
#include <span>
namespace RetroMediaLab::MediaEngine::ScpInspection::Constants {
inline constexpr std::size_t MaximumTrackCount=24;
inline constexpr double MinimumDecodedFormatScore=0.4,MinimumCylinderRatio=0.9;
inline constexpr std::uint32_t CompatibleCylinders=80,CompatibleHeads=2,CompatibleBlockSize=512,CompatibleSectors=9;
inline constexpr std::array AmigaCodecs{AmigaMfm::Constants::Id};
inline constexpr std::array IsoCodecs{IsoMfm::Constants::Id,IsoFm::Constants::Id};
inline constexpr std::array CommodoreCodecs{Commodore1541Gcr::Constants::Id};
inline constexpr std::array AppleCodecs{AppleII::Constants::Id,AppleRwts18::Constants::Id,AppleIwm::Constants::MacId};
inline constexpr std::array DecCodecs{DecRx02::Constants::Id};
struct ProbeDefinition {ScpReconstruction::Family family;std::span<const std::string_view> decoderIds;};
inline constexpr std::array Probes{ProbeDefinition{ScpReconstruction::Family::Amiga,AmigaCodecs},ProbeDefinition{ScpReconstruction::Family::Iso,IsoCodecs},ProbeDefinition{ScpReconstruction::Family::Commodore,CommodoreCodecs},ProbeDefinition{ScpReconstruction::Family::Apple,AppleCodecs},ProbeDefinition{ScpReconstruction::Family::Dec,DecCodecs}};
}
