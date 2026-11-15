#pragma once
#include "AdfImages.h"
#include "Constants.h"
namespace RetroMediaLab::MediaEngine::Adf::Internal {
bool same(std::string_view,std::string_view);
std::uint64_t capacity(const Profile&);
Profile profile(const Constants::Specification&);
bool acceptedSize(std::uint64_t);
std::string acceptedSizes();
std::vector<std::uint8_t> header(const MediaFileSystems::RandomAccessData&,std::stop_token);
std::shared_ptr<const MediaFileSystems::SectorImage> image(std::shared_ptr<const MediaFileSystems::RandomAccessData>,const Profile&,std::optional<std::uint64_t> capacity = {});
std::shared_ptr<const MediaFileSystems::SectorImage> partial(std::shared_ptr<const MediaFileSystems::RandomAccessData>,MediaFileSystems::Bytes,std::stop_token);
}
