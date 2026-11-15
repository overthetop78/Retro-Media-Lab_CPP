#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include <filesystem>
#include <RetroMediaLab/MediaFileSystems/FileSystemModels.h>
#include <span>
#include <string_view>
namespace RetroMediaLab::MediaEngine::CommodoreDisks {
enum class Kind {D64,D71,D81};
enum class ErrorMapMode {None,Preserve};
struct Layout {std::string_view nameKey;std::uint64_t imageLength=0;std::uint32_t tracksPerSide=0,sides=0,dataBlockCount=0;std::optional<std::uint64_t> errorMapOffset;};
RETROMEDIALAB_MEDIAENGINE_API std::span<const Layout> layouts(Kind);
RETROMEDIALAB_MEDIAENGINE_API const Layout* findLayout(Kind,std::uint64_t length);
RETROMEDIALAB_MEDIAENGINE_API std::string_view formatId(Kind);
RETROMEDIALAB_MEDIAENGINE_API bool canRead(Kind,const MediaFileSystems::RandomAccessData&,std::optional<std::string_view> requested={},std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> read(Kind,std::shared_ptr<const MediaFileSystems::RandomAccessData>,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API void writeD64D71(const MediaFileSystems::SectorImage&,const std::filesystem::path&,ErrorMapMode = ErrorMapMode::None,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API bool canWriteD81(const MediaFileSystems::SectorImage&);
RETROMEDIALAB_MEDIAENGINE_API void writeD81(const MediaFileSystems::SectorImage&,const std::filesystem::path&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API MediaFileSystems::FileSystemVolume explore(Kind,std::shared_ptr<const MediaFileSystems::RandomAccessData>,std::stop_token={});
}
