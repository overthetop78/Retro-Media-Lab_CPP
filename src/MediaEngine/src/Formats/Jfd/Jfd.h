#pragma once
#include "../../Models/Sectors/SectorImages.h"
#include <span>
#include <stop_token>
namespace RetroMediaLab::MediaEngine::Jfd {
struct Record {
    std::uint32_t track{},order{},header{};
    std::optional<std::size_t> offset;
    std::size_t length{},descriptorOffset{};
    bool rawTrack() const noexcept;
};
struct Image {
    std::vector<std::uint8_t> bytes;
    std::vector<Record> records;
    std::shared_ptr<const MediaFileSystems::SectorImage> sectors;
    bool hasRawTracks=false,hasRotationalPositions=false;
};
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> unpack(std::span<const std::uint8_t>,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API Image decode(std::span<const std::uint8_t>,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> rewrite(const Image&,const MediaFileSystems::SectorImage&,std::stop_token = {});
}
