#include "Geometry.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>

namespace RetroMediaLab::MediaEngine::Apple::Geometry {
namespace {
constexpr bool permutation(const auto& table) {
    for(std::int32_t value=0;value<AppleII::SectorsPerTrack;++value)
        if(std::count(table.begin(),table.end(),value)!=1)return false;
    return true;
}
static_assert(permutation(AppleII::ProDosToPhysical)&&permutation(AppleII::PhysicalToDos));
std::int32_t zoneSectors(std::int32_t cylinder,const auto& ends,const auto& counts) {
    if(cylinder<0||cylinder>=ends.back())throw MediaError(DiagnosticCode::AppleGeometryCylinder,{std::to_string(cylinder),std::to_string(ends.back()-1)});
    for(std::size_t zone=0;zone<ends.size();++zone)if(cylinder<ends[zone])return counts[zone];
    return counts.back();
}
void requireMacHeads(std::int32_t heads) {
    if(heads!=SingleHead&&heads!=DoubleHeads)throw MediaError(DiagnosticCode::AppleGeometryHeads,{std::to_string(heads),std::to_string(SingleHead),std::to_string(DoubleHeads)});
}
constexpr std::array D13Kinds{Kind::AppleII525Dos32};
constexpr std::array AppleIIKinds{Kind::AppleII525Dos33,Kind::AppleII525ProDos};
constexpr std::array Apple400Kinds{Kind::LisaFileWare,Kind::MacintoshGcr400};
constexpr std::array Apple800Kinds{Kind::MacintoshGcr800,Kind::ProDos800};
constexpr std::array Macintosh1440Kinds{Kind::MacintoshMfm1440};
constexpr std::array Catalogue{
    Layout{LayoutNames::D13,AppleII::Dos32Capacity,D13Kinds},
    Layout{LayoutNames::AppleII140K,AppleII::Capacity,AppleIIKinds},
    Layout{LayoutNames::Apple400K,MacintoshGcr::Capacity400K,Apple400Kinds},
    Layout{LayoutNames::Apple800K,MacintoshGcr::Capacity800K,Apple800Kinds},
    Layout{LayoutNames::Macintosh1440K,MacintoshMfm::Capacity,Macintosh1440Kinds}};
}
std::int32_t MacintoshGcr::sectors(std::int32_t cylinder){return zoneSectors(cylinder,ZoneEnds,ZoneSectors);}
MacintoshGcr::ImageGeometry MacintoshGcr::forHeads(std::int32_t heads){
    requireMacHeads(heads);return {heads,SingleSidedBlockCount*heads,Capacity400K*heads};
}
bool MacintoshGcr::isSupportedCapacity(std::int32_t capacity) noexcept{return capacity==Capacity400K||capacity==Capacity800K||capacity==Capacity1440K;}
MediaFileSystems::SectorAddress MacintoshGcr::address(std::int32_t logicalBlock,std::int32_t heads){
    const auto geometry=forHeads(heads);
    if(logicalBlock<0||logicalBlock>=geometry.blockCount)throw MediaError(DiagnosticCode::AppleGeometryMacBlock,{std::to_string(logicalBlock),std::to_string(geometry.blockCount),std::to_string(heads)});
    auto remaining=logicalBlock;
    for(std::int32_t cylinder=0;cylinder<CylinderCount;++cylinder){
        const auto count=sectors(cylinder);
        if(remaining<count*heads)return {cylinder,remaining/count,remaining%count};
        remaining-=count*heads;
    }
    throw MediaError(DiagnosticCode::AppleGeometryMacBlock,{std::to_string(logicalBlock),std::to_string(geometry.blockCount),std::to_string(heads)});
}
std::int32_t LisaFileWare::sectors(std::int32_t cylinder){return zoneSectors(cylinder,ZoneEnds,ZoneSectors);}
MediaFileSystems::SectorAddress LisaFileWare::address(std::int32_t logicalBlock){
    if(logicalBlock<0||logicalBlock>=BlockCount)throw MediaError(DiagnosticCode::AppleGeometryBlock,{std::to_string(logicalBlock),std::to_string(BlockCount)});
    constexpr auto sectorsPerSide=BlockCount/HeadCount;
    const auto head=logicalBlock/sectorsPerSide;
    auto remaining=logicalBlock%sectorsPerSide;
    for(std::int32_t cylinder=0;cylinder<CylinderCount;++cylinder){
        const auto count=sectors(cylinder);
        if(remaining<count)return {cylinder,head,remaining};
        remaining-=count;
    }
    throw MediaError(DiagnosticCode::AppleGeometryBlock,{std::to_string(logicalBlock),std::to_string(BlockCount)});
}
std::span<const Layout> layouts() noexcept{return Catalogue;}
const Layout* findLayout(std::int32_t capacity) noexcept{
    for(const auto& layout:Catalogue)if(layout.capacity==capacity)return &layout;
    return nullptr;
}
}
