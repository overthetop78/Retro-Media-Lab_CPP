#pragma once
#include "CandidateDecoder.h"
#include "Policy.h"
namespace RetroMediaLab::MediaEngine::IsoReconstruction {
// Synchronous worker service. Decode operations own their registry state; callbacks may run concurrently.
class Reader final {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit Reader(DecodeFlux,ReadCapture={});
    RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> read(const std::filesystem::path&,std::optional<std::string> format={},const CollectionProgressCallback& progress={},std::stop_token={});
    RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> read(const Flux::ProtectedTrackImage&,std::optional<std::string> format={},const CollectionProgressCallback& progress={},std::stop_token={});
    RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::SectorImage> readAtari(const std::filesystem::path&,std::optional<std::string> format={},const CollectionProgressCallback& progress={},std::stop_token={});
private:
    CandidateDecoder candidates_;
};
}
