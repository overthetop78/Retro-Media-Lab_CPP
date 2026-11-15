#pragma once
#include "Collector.h"
#include "../../Formats/Scp/Scp.h"
#include <filesystem>
#include <memory>
namespace RetroMediaLab::MediaEngine::IsoReconstruction {
using SharedCandidates=std::shared_ptr<const CandidateSet>;
using ReadCapture=std::function<Scp::Image(const std::filesystem::path&,std::stop_token)>;
// Calls may run concurrently. Supplied decoder, reader and progress callbacks must be thread-safe.
// A cache hit shares the first call's result, progress receiver and cancellation token.
class CandidateDecoder final {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit CandidateDecoder(DecodeFlux,ReadCapture = {});
    RETROMEDIALAB_MEDIAENGINE_API ~CandidateDecoder();
    CandidateDecoder(const CandidateDecoder&)=delete;
    CandidateDecoder& operator=(const CandidateDecoder&)=delete;
    RETROMEDIALAB_MEDIAENGINE_API SharedCandidates decode(const std::filesystem::path&,std::span<const std::string>,const CollectionProgressCallback& = {},std::stop_token = {});
    RETROMEDIALAB_MEDIAENGINE_API SharedCandidates decode(const Flux::ProtectedTrackImage&,std::span<const std::string>,const CollectionProgressCallback& = {},std::stop_token = {}) const;
private:
    class State;
    std::unique_ptr<State> state_;
};
}
