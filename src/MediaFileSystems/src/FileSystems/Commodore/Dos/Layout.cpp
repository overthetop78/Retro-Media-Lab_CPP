#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::CommodoreDos {
std::string_view baseTypeName(std::uint8_t type) noexcept {const auto base=std::uint8_t(type&Constants::BaseTypeMask);return Constants::TypeNames[base<Constants::TypeNames.size()?base:0];}
namespace Internal {
std::optional<Layout> layout(std::string_view id) {
    using namespace Constants;namespace G=Commodore::Constants;
    if(id==G::Format1541||id==G::Format1571)return Layout{HeaderTrack1541,HeaderSector1541,HeaderTrack1541,DirectorySector1541,Name1541,Signature1541,false};
    if(id==G::Format1581)return Layout{HeaderTrack1581,HeaderSector1581,HeaderTrack1581,DirectorySector1581,Name1581,Signature1581,true};
    return {};
}
}
}
