#include "DiskTypes.h"
#include "DiskTypeConstants.h"
#include "../FormatIds.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaFileSystems/CommodoreGeometryConstants.h>
#include <array>
namespace RetroMediaLab::MediaEngine::Scp {
std::uint8_t diskType(const MediaFileSystems::SectorImage& image) {
    using namespace DiskTypes;
    using Primitives::asciiIdentifierEqualsIgnoreCase;
    using Primitives::asciiIdentifierStartsWithIgnoreCase;
    const auto& g=image.geometry();const auto& id=g.formatId;
    namespace Cbm=MediaFileSystems::Commodore::Constants;
    if(asciiIdentifierEqualsIgnoreCase(id,Cbm::Format1541)||asciiIdentifierEqualsIgnoreCase(id,Cbm::Format1571))return Commodore64;
    if(asciiIdentifierStartsWithIgnoreCase(id,Formats::Commodore900Prefix))return Other1200;
    if(asciiIdentifierEqualsIgnoreCase(id,Formats::AmigaDosHighDensity))return AmigaHighDensity;
    if(asciiIdentifierStartsWithIgnoreCase(id,Formats::AmigaPrefix))return Amiga;
    if(asciiIdentifierEqualsIgnoreCase(id,Formats::Atari90)||asciiIdentifierEqualsIgnoreCase(id,Formats::AtariXfd90))return Atari8BitSingleDensity;
    if(asciiIdentifierEqualsIgnoreCase(id,Formats::Atari130)||asciiIdentifierEqualsIgnoreCase(id,Formats::AtariXfd130))return Atari8BitEnhancedDensity;
    if(asciiIdentifierStartsWithIgnoreCase(id,Formats::AtariPrefix))return Atari8BitDoubleDensity;
    if(asciiIdentifierStartsWithIgnoreCase(id,Formats::AtariStPrefix))return g.heads==SingleHead?AtariStSingleSided:AtariStDoubleSided;
    if(asciiIdentifierEqualsIgnoreCase(id,Formats::AppleIIProDos800))return AppleMacintosh800;
    if(asciiIdentifierStartsWithIgnoreCase(id,Formats::AppleIIPrefix)||asciiIdentifierStartsWithIgnoreCase(id,Formats::AppleIIIPrefix)){
        for(std::size_t index=0;index<id.size();++index)
            if(asciiIdentifierStartsWithIgnoreCase(std::string_view(id).substr(index),ProDosKeyword))return AppleIIProDos;
        return AppleII;
    }
    if(asciiIdentifierEqualsIgnoreCase(id,Formats::Mac1440))return AppleMacintosh1440;
    if(asciiIdentifierStartsWithIgnoreCase(id,Formats::AppleMacPrefix)||asciiIdentifierStartsWithIgnoreCase(id,Formats::MacPrefix))return g.heads==SingleHead?AppleMacintosh400:AppleMacintosh800;
    constexpr std::array lowDensity{Formats::Ibm160,Formats::Ibm180,Formats::Ibm320,Formats::Ibm360};
    for(auto value:lowDensity)if(asciiIdentifierEqualsIgnoreCase(id,value))return IbmPc360;
    if(asciiIdentifierEqualsIgnoreCase(id,Formats::Ibm720)||asciiIdentifierEqualsIgnoreCase(id,Formats::Ibm800))return IbmPc720;
    if(asciiIdentifierEqualsIgnoreCase(id,Formats::Ibm1200))return IbmPc1200;
    constexpr std::array highDensity{Formats::Ibm1440,Formats::Ibm1680,Formats::IbmDmf,Formats::Ibm2880};
    for(auto value:highDensity)if(asciiIdentifierEqualsIgnoreCase(id,value))return IbmPc1440;
    if(asciiIdentifierEqualsIgnoreCase(id,Formats::AmstradCpc))return AmstradCpc;
    const auto capacity=image.capacity()/BytesPerKibibyte;
    if(capacity==Capacity720||capacity==Capacity800||capacity==Capacity880)return Other720;
    if(capacity==Capacity1200)return Other1200;
    if(capacity==Capacity1440)return Other1440;
    return Other320;
}
}

