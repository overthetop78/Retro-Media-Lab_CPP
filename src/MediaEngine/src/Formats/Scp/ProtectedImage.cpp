#include "ProtectedImage.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Scp {
Flux::ProtectedTrackImage protectedImage(std::shared_ptr<const Image> image,std::stop_token token){
    checkCancellation(token);if(!image)throw MediaError(DiagnosticCode::ScpArgument,{std::string(Constants::ImageArgument)});
    std::vector<Flux::ProtectedTrack> tracks;tracks.reserve(image->tracks.size());
    for(const auto& track:image->tracks){
        checkCancellation(token);std::vector<Flux::TrackFluxRevolution> revolutions;revolutions.reserve(track.revolutions.size());
        for(const auto& revolution:track.revolutions){checkCancellation(token);revolutions.emplace_back(image->header.resolutionNanoseconds(),Flux::SharedRevolution(image,&revolution.flux));}
        tracks.emplace_back(track.cylinder,track.head,std::nullopt,std::vector<Flux::TrackTimingSegment>{},std::vector<Decoding::Structure>{},std::vector<Flux::TrackFeature>{},std::move(revolutions));
    }
    return {std::move(tracks),(image->header.flags&Constants::WritableFlag)==0};
}
Flux::ProtectedTrackImage protectedImage(const Image& image,std::stop_token token){
    checkCancellation(token);std::vector<Flux::ProtectedTrack> tracks;tracks.reserve(image.tracks.size());
    for(const auto& track:image.tracks){
        checkCancellation(token);std::vector<Flux::TrackFluxRevolution> revolutions;revolutions.reserve(track.revolutions.size());
        for(const auto& revolution:track.revolutions){checkCancellation(token);revolutions.emplace_back(image.header.resolutionNanoseconds(),revolution.flux);}
        tracks.emplace_back(track.cylinder,track.head,std::nullopt,std::vector<Flux::TrackTimingSegment>{},std::vector<Decoding::Structure>{},std::vector<Flux::TrackFeature>{},std::move(revolutions));
    }
    checkCancellation(token);return {std::move(tracks),(image.header.flags&Constants::WritableFlag)==0};
}
}
