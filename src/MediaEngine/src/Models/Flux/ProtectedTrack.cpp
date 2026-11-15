#include "ProtectedTrack.h"
#include "ProtectedConstants.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <cmath>
namespace RetroMediaLab::MediaEngine::Flux {
TrackFluxRevolution::TrackFluxRevolution(std::int32_t resolution,Revolution flux):resolution_(resolution),flux_(std::move(flux)){
    if(resolution<=0)throw MediaError(DiagnosticCode::ProtectedResolution,{std::to_string(resolution)});
}
namespace {
void range(std::int64_t offset,std::int64_t length,std::uint64_t count,std::string_view field){
    if(offset<0||length<=0||std::uint64_t(offset)>count||std::uint64_t(length)>count-std::uint64_t(offset))throw MediaError(DiagnosticCode::ProtectedRange,{std::string(field),std::to_string(count)});
}
}
ProtectedTrack::ProtectedTrack(std::int32_t cylinder,std::int32_t head,std::optional<Bits> bits,std::vector<TrackTimingSegment> timing,std::vector<Decoding::Structure> structures,std::vector<TrackFeature> features,std::vector<TrackFluxRevolution> revolutions)
    :cylinder_(cylinder),head_(head),bits_(std::move(bits)),timing_(std::move(timing)),structures_(std::move(structures)),features_(std::move(features)),revolutions_(std::move(revolutions)){
    if(cylinder<0)throw MediaError(DiagnosticCode::ProtectedCylinder,{std::to_string(cylinder)});
    if(head<0)throw MediaError(DiagnosticCode::ProtectedHead,{std::to_string(head)});
    if((!bits_||bits_->empty())&&revolutions_.empty())throw MediaError(DiagnosticCode::ProtectedNoContent);
    if(!bits_&&(!timing_.empty()||!structures_.empty()||!features_.empty()))throw MediaError(DiagnosticCode::ProtectedMetadataNoBits);
    if(bits_){
        if(bits_->size()>Constants::MaximumBits)throw MediaError(DiagnosticCode::FluxSizeLimit);
        if(std::any_of(bits_->begin(),bits_->end(),[](auto bit){return bit>1;}))throw MediaError(DiagnosticCode::FluxInvalidBit);
        for(const auto& segment:timing_)range(segment.bitOffset,segment.bitLength,bits_->size(),ProtectedConstants::TimingField);
        for(const auto& structure:structures_)range(std::int64_t(structure.bitOffset),std::int64_t(structure.bitLength),bits_->size(),ProtectedConstants::StructuresField);
        for(const auto& feature:features_)range(feature.bitOffset,feature.bitLength,bits_->size(),ProtectedConstants::FeaturesField);
        for(const auto& segment:timing_)if(!std::isfinite(segment.cellNanoseconds)||segment.cellNanoseconds<=0)throw MediaError(DiagnosticCode::ProtectedTiming);
    }
}
}
