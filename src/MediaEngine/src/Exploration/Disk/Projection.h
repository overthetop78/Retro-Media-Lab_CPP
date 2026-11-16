#pragma once
#include "Explorer.h"
#include <RetroMediaLab/MediaEngine/DiskExploration.h>
#include <map>
namespace RetroMediaLab::MediaEngine::DiskProjection {
using Block=std::shared_ptr<const MediaFileSystems::SectorBlock>;
struct ImageBlocks {std::map<std::uint64_t,Block> values;std::vector<std::uint64_t> order;};
struct Cache {
 std::map<const MediaFileSystems::SectorImage*,ImageBlocks> images;
 Block block(const MediaFileSystems::SectorImage&,std::uint64_t)const;
};
std::shared_ptr<const Cache> load(const DiskExploration&,std::stop_token);
DiskContainerInfo container(const DiskExploration&);
std::vector<DiskTrackInfo> tracks(const DiskExploration&,const Cache&);
std::vector<DiskFormatInfo> formats(const DiskExploration&,const Cache&,std::stop_token);
std::shared_ptr<const FileSystemVolumeInfo> volume(const ScpInspection::MatchOwner&);
std::shared_ptr<const FileSystemVolumeInfo> physicalVolume(const DiskExploration&);
DiskImageInfo project(const DiskExploration&,const Cache&,std::stop_token);
}
namespace RetroMediaLab::MediaEngine {
class DiskProjectionAccess final {
public:
 RETROMEDIALAB_MEDIAENGINE_API static std::shared_ptr<const ExploredDisk> create(DiskExploration,std::stop_token={});
 RETROMEDIALAB_MEDIAENGINE_API static std::shared_ptr<const DiskCaptureSnapshot> capture(Scp::SharedImage);
};
}
