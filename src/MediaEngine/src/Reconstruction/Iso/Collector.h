#pragma once
#include "Models.h"
#include "../../Decoding/Decoder.h"
#include "../../Models/Flux/ProtectedTrack.h"
#include <functional>
#include <span>
#include <stop_token>
namespace RetroMediaLab::MediaEngine::IsoReconstruction {
using DecodeFlux=Decoding::DecodeOperation;
enum class CollectionProgressKind {RevolutionDecoded,TrackDecoded};
struct CollectionProgress {
    CollectionProgressKind kind;std::int32_t completed,total,cylinder,head,revolution,trackRevolutions;
};
// Called on worker threads. The receiver must synchronize its own state.
using CollectionProgressCallback=std::function<void(const CollectionProgress&)>;
RETROMEDIALAB_MEDIAENGINE_API double score(const Decoding::Result&) noexcept;
RETROMEDIALAB_MEDIAENGINE_API CandidateSet collectTrack(const Flux::ProtectedTrack&,std::span<const std::string>,const DecodeFlux&,const std::function<void(std::int32_t)>& = {},std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API CandidateSet collect(const Flux::ProtectedTrackImage&,std::span<const std::string>,const DecodeFlux&,const CollectionProgressCallback& = {},std::stop_token = {});
}
