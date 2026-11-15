#include "Images.h"
#include "Constants.h"
#include <RetroMediaLab/MediaFileSystems/CommodoreGeometry.h>
namespace RetroMediaLab::MediaEngine::CommodoreDisks {
namespace {
using namespace MediaFileSystems::Commodore;
std::array<Layout,Constants::NameKeys.size()> create(std::uint32_t sides) {
    std::array<Layout,Constants::NameKeys.size()> result{};std::size_t index=0;
    for(const auto tracks:MediaFileSystems::Commodore::Constants::TrackCounts)for(const auto errors:{false,true}) {
        const auto blocks=std::uint32_t(blocksPerSide1541(tracks))*sides;
        const auto length=std::uint64_t(blocks)*MediaFileSystems::Commodore::Constants::SectorSize;
        const auto key=(sides-1)*MediaFileSystems::Commodore::Constants::TwoSides+std::uint32_t(errors);
        result[index++]={Constants::NameKeys[key],length+(errors?blocks:0),std::uint32_t(tracks),sides,blocks,errors?std::optional<std::uint64_t>(length):std::nullopt};
    }
    return result;
}
}
std::span<const Layout> layouts(Kind kind) {
    static const auto d64=create(1),d71=create(MediaFileSystems::Commodore::Constants::TwoSides);
    static const std::array<Layout,1> d81{{{{},std::uint64_t(MediaFileSystems::Commodore::Constants::Cylinders1581)*MediaFileSystems::Commodore::Constants::BlocksPerTrack1581*MediaFileSystems::Commodore::Constants::SectorSize,MediaFileSystems::Commodore::Constants::Cylinders1581,MediaFileSystems::Commodore::Constants::Heads1581,MediaFileSystems::Commodore::Constants::Cylinders1581*MediaFileSystems::Commodore::Constants::BlocksPerTrack1581,{}}}};
    switch(kind){case Kind::D64:return d64;case Kind::D71:return d71;case Kind::D81:return d81;}return {};
}
const Layout* findLayout(Kind kind,std::uint64_t length) {for(const auto& layout:layouts(kind))if(layout.imageLength==length)return &layout;return nullptr;}
std::string_view formatId(Kind kind) {using namespace MediaFileSystems::Commodore;switch(kind){case Kind::D64:return MediaFileSystems::Commodore::Constants::Format1541;case Kind::D71:return MediaFileSystems::Commodore::Constants::Format1571;case Kind::D81:return MediaFileSystems::Commodore::Constants::Format1581;}return {};}
}
