#include "Representation.h"
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaEngine {
namespace M=MediaFileSystems;
FluxRepresentation::FluxRepresentation(std::shared_ptr<const Flux::ProtectedTrackImage> image):image_(std::move(image)){
    if(!image_)throw M::MediaError(M::DiagnosticCode::RepresentationImageNull);
    surfaces_.reserve(image_->tracks().size());for(const auto& track:image_->tracks())surfaces_.push_back(track.head());std::sort(surfaces_.begin(),surfaces_.end());surfaces_.erase(std::unique(surfaces_.begin(),surfaces_.end()),surfaces_.end());
}
std::int32_t FluxRepresentation::revolutionCount()const{std::int32_t count=0;for(const auto& track:image_->tracks()){if(track.revolutions().size()>std::size_t(std::numeric_limits<std::int32_t>::max()-count))throw M::MediaError(M::DiagnosticCode::RepresentationCountOverflow);count+=std::int32_t(track.revolutions().size());}return count;}
std::int64_t FluxRepresentation::transitionCount()const{std::int64_t count=0;for(const auto& track:image_->tracks()){std::int32_t perTrack=0;for(const auto& revolution:track.revolutions()){if(revolution.flux().intervals.size()>std::size_t(std::numeric_limits<std::int32_t>::max()-perTrack))throw M::MediaError(M::DiagnosticCode::RepresentationCountOverflow);perTrack+=std::int32_t(revolution.flux().intervals.size());}if(perTrack>std::numeric_limits<std::int64_t>::max()-count)throw M::MediaError(M::DiagnosticCode::RepresentationCountOverflow);count+=perTrack;}return count;}
}
