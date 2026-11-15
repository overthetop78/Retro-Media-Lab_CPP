#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <cstdint>
#include <filesystem>
#include <map>
#include <optional>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <vector>
namespace RetroMediaLab::MediaEngine::Definitions {
struct IdentifierLess {
    using is_transparent=void;
    RETROMEDIALAB_MEDIAENGINE_API bool operator()(std::string_view,std::string_view)const;
};
using Parameters=std::map<std::string,std::string,IdentifierLess>;
struct TrackDefinition {
    std::string selection,codec;
    Parameters parameters;
    RETROMEDIALAB_MEDIAENGINE_API std::int32_t number(std::string_view,std::int32_t fallback)const;
    RETROMEDIALAB_MEDIAENGINE_API bool matches(std::int32_t cylinder,std::int32_t head)const;
};
struct Sector {
    std::uint64_t logicalBlock=0;
    std::int32_t cylinder=0,head=0,number=0,size=0,imageSize=0;
    std::size_t trackIndex=0;
};
struct DiskDefinition {
    std::string id;
    std::int32_t cylinders=0,heads=0;
    std::vector<TrackDefinition> tracks;
    RETROMEDIALAB_MEDIAENGINE_API const TrackDefinition* track(std::int32_t cylinder,std::int32_t head)const;
    RETROMEDIALAB_MEDIAENGINE_API std::vector<Sector> layout()const;
};
using Catalogue=std::map<std::string,DiskDefinition,IdentifierLess>;
using Identifiers=std::set<std::string,IdentifierLess>;
RETROMEDIALAB_MEDIAENGINE_API const Catalogue& embedded();
RETROMEDIALAB_MEDIAENGINE_API Catalogue rawDefinitions(std::span<const std::string> curatedIds);
RETROMEDIALAB_MEDIAENGINE_API Identifiers readFileIds(const std::filesystem::path&);
RETROMEDIALAB_MEDIAENGINE_API std::optional<std::pair<std::int32_t,std::int32_t>> dimensions(std::string_view);
// Numeric parameters use the C# invariant decimal integer syntax and checked int32 range.
RETROMEDIALAB_MEDIAENGINE_API std::int32_t parseNumber(std::string_view field,std::string_view value);
}
