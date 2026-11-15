#pragma once
#include "Models.h"
#include <functional>
#include <string>
namespace RetroMediaLab::MediaEngine::IsoReconstruction {
RETROMEDIALAB_MEDIAENGINE_API const Candidate& best(std::span<const Candidate>,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> bestData(const Candidates&,const MediaFileSystems::SectorAddress&,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API Measurement measure(const Candidates&,std::stop_token = {});
struct UniformRequest {
    std::string formatId;
    std::int32_t sectorSize=0,cylinders=0,heads=0,sectorsPerTrack=0;
    std::function<std::int32_t(const MediaFileSystems::SectorAddress&)> sectorIndex;
    bool allowSectorNumbersBeyondGeometry=false,allowVariableBlockSize=false;
    std::optional<std::int64_t> capacity;
    std::function<std::vector<std::uint8_t>(std::span<const std::uint8_t>)> normalizeData;
};
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> createUniform(const Candidates&,const UniformRequest&,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> padTo(std::span<const std::uint8_t>,std::int32_t size,std::stop_token = {});
}
