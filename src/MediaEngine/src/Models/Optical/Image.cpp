#include "Image.h"
#include "Internal.h"
#include "Constants.h"
#include "../../Sources/UnavailableRandomAccessData.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace M=MediaFileSystems;
OpticalImage::OpticalImage(OpticalImageOptions value):options_(std::move(value)){
    if(options_.logicalLength&&*options_.logicalLength<0)throw M::MediaError(M::DiagnosticCode::OpticalLogicalLength);
    if(options_.layerCount&&*options_.layerCount<=0)throw M::MediaError(M::DiagnosticCode::OpticalPositiveDimension,{std::string(OpticalConstants::Layers)});
    if(options_.faceCount&&*options_.faceCount<=0)throw M::MediaError(M::DiagnosticCode::OpticalPositiveDimension,{std::string(OpticalConstants::Faces)});
    if(options_.tracks){tracks_.emplace();sessions_.emplace();tracks_->reserve(options_.tracks->size());for(const auto& track:*options_.tracks){if(!track)throw M::MediaError(M::DiagnosticCode::OpticalNullTrack);tracks_->push_back(track);sessions_->push_back(track->sessionNumber());}std::sort(sessions_->begin(),sessions_->end());sessions_->erase(std::unique(sessions_->begin(),sessions_->end()),sessions_->end());}
}
std::shared_ptr<const OpticalImage> OpticalImage::fromCoordinates(std::optional<std::int64_t> length,std::vector<OpticalTrackCoordinates> tracks,std::optional<std::int32_t> layers,std::optional<std::int32_t> faces,std::optional<std::vector<std::string>> files){
    OpticalImageOptions options;options.logicalLength=length;options.layerCount=layers;options.faceCount=faces;options.associatedFiles=std::move(files);options.tracks.emplace();options.tracks->reserve(tracks.size());
    for(const auto& track:tracks){if(track.sectorCount<0)throw M::MediaError(M::DiagnosticCode::OpticalLogicalLength);const auto count=OpticalInternal::multiply(track.sectorCount,OpticalConstants::CompatibilitySectorSize);OpticalTrackOptions value;value.sessionNumber=track.sessionNumber;value.trackNumber=track.trackNumber;value.mode=OpticalTrackMode::Mode1Data2048;value.firstSector=track.firstSector;value.sectorCount=track.sectorCount;value.storedSectorSize=value.userDataLength=OpticalConstants::CompatibilitySectorSize;value.dataSource=std::make_shared<UnavailableRandomAccessData>(count);options.tracks->push_back(std::make_shared<OpticalTrack>(std::move(value)));}
    return std::make_shared<OpticalImage>(std::move(options));
}
}

