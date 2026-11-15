#pragma once
#include "Images.h"
#include "../Scp/Scp.h"
#include <filesystem>
namespace RetroMediaLab::MediaEngine::CommodoreDisks {
using TrackProgress=std::function<void(const Scp::Track&,std::size_t completed,std::size_t total)>;
// Exact source format selection; absent or other format IDs use the GCR reconstruction.
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> readFlux(const Scp::Image&,std::optional<std::string_view> requested={},TrackProgress={},std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> readFlux(const std::filesystem::path&,std::optional<std::string_view> requested={},TrackProgress={},std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> readMfmFlux(const Scp::Image&,TrackProgress={},std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> readGcrFlux(const Scp::Image&,std::optional<std::string_view> requested={},TrackProgress={},std::stop_token={});
}
