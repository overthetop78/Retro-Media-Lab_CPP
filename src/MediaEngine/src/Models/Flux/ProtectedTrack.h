#pragma once
#include "Flux.h"
#include "../../Decoding/Models.h"
namespace RetroMediaLab::MediaEngine::Flux {
enum class TrackFeatureKind {IndexMark,Gap,IntentionalChecksumError,WeakRegion};
struct TrackTimingSegment {std::int64_t bitOffset=0,bitLength=0;double cellNanoseconds=0;};
struct TrackFeature {TrackFeatureKind kind;std::int64_t bitOffset=0,bitLength=0;std::optional<Decoding::LocalizedText> description;};
class TrackFluxRevolution {
public:
    RETROMEDIALAB_MEDIAENGINE_API TrackFluxRevolution(std::int32_t resolutionNanoseconds,Revolution);
    std::int32_t resolutionNanoseconds() const noexcept{return resolution_;}
    const Revolution& flux() const noexcept{return flux_;}
private:
    std::int32_t resolution_;Revolution flux_;
};
class ProtectedTrack {
public:
    RETROMEDIALAB_MEDIAENGINE_API ProtectedTrack(std::int32_t cylinder,std::int32_t head,std::optional<Bits>,std::vector<TrackTimingSegment>,std::vector<Decoding::Structure>,std::vector<TrackFeature>,std::vector<TrackFluxRevolution>);
    std::int32_t cylinder() const noexcept{return cylinder_;}
    std::int32_t head() const noexcept{return head_;}
    const std::optional<Bits>& bits() const noexcept{return bits_;}
    const std::vector<TrackTimingSegment>& timing() const noexcept{return timing_;}
    const std::vector<Decoding::Structure>& structures() const noexcept{return structures_;}
    const std::vector<TrackFeature>& features() const noexcept{return features_;}
    const std::vector<TrackFluxRevolution>& revolutions() const noexcept{return revolutions_;}
private:
    std::int32_t cylinder_,head_;std::optional<Bits> bits_;std::vector<TrackTimingSegment> timing_;std::vector<Decoding::Structure> structures_;std::vector<TrackFeature> features_;std::vector<TrackFluxRevolution> revolutions_;
};
class ProtectedTrackImage {
public:
    RETROMEDIALAB_MEDIAENGINE_API ProtectedTrackImage(std::vector<ProtectedTrack>,bool writeProtected);
    const std::vector<ProtectedTrack>& tracks() const noexcept{return tracks_;}
    bool writeProtected() const noexcept{return writeProtected_;}
private:
    std::vector<ProtectedTrack> tracks_;bool writeProtected_;
};
}
