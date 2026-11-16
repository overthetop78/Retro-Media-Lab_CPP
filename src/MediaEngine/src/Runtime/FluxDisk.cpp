#include "PhysicalStates.h"
#include "../Models/Flux/Conversion.h"
#include <QString>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace M=MediaFileSystems;
RuntimeDisk::FluxState::FluxState(std::shared_ptr<RuntimeDetails::Services> engine,std::shared_ptr<const MediaDocument> source,bool allowed,bool owns):State(std::move(engine),std::move(source),allowed,owns){
    const auto* image=dynamic_cast<const FluxRepresentation*>(&document->representation());if(!image||image->tracks().empty())throw MediaError(DiagnosticCode::RuntimeDiskRequest);
    readOnly=readOnly||image->image().writeProtected();
    for(const auto& track:image->tracks()){
        if(!tracks.emplace(Address{track.cylinder(),track.head()},track).second)throw MediaError(DiagnosticCode::RuntimeTrackData);
        cylinders_=std::max(cylinders_,std::int64_t(track.cylinder())+1);heads_=std::max(heads_,RuntimeDetails::coordinate(std::int64_t(track.head())+1));revolutionCount=std::max(revolutionCount,track.revolutions().size());
        readOnly=readOnly||std::any_of(track.features().begin(),track.features().end(),[](const auto& feature){return feature.kind==Flux::TrackFeatureKind::WeakRegion;});
    }
}
RuntimeDisk::FluxState::~FluxState()noexcept{try{close();}catch(...) {}}
void RuntimeDisk::FluxState::validateAddress(std::int32_t cylinder,std::int32_t head)const{alive();if(cylinder<0||cylinder>=cylinders_||head<0||head>=heads_)throw MediaError(DiagnosticCode::RuntimeTrackWindow);}
FluxTrackData RuntimeDisk::FluxState::readTrack(std::int32_t cylinder,std::int32_t head,std::string_view encoding,std::uint32_t cell,std::uint32_t period,const Attributes& attributes){
    validateAddress(cylinder,head);if(!cell||!period)throw MediaError(DiagnosticCode::RuntimeTrackWindow);(void)encoding;(void)attributes;const Address address{cylinder,head};
    if(const auto found=cache.find(address);found!=cache.end())return found->second;
    const auto found=tracks.find(address);std::vector<FluxRevolutionData> values;
    if(found==tracks.end()){for(std::size_t i=0;i<revolutionCount;++i)values.emplace_back(period,std::vector<std::uint32_t>{});return FluxTrackData(std::move(values));}
    for(auto& r:Flux::readFlux(found->second))values.emplace_back(r.indexTime,std::move(r.intervals));
    auto result=FluxTrackData(std::move(values));cache.emplace(address,result);return result;
}
void RuntimeDisk::FluxState::writeTrackWindow(std::int32_t cylinder,std::int32_t head,std::string_view encoding,std::uint32_t cell,std::uint32_t period,std::uint32_t start,std::uint32_t duration,std::span<const std::uint32_t> offsets,const Attributes& attributes,bool ending){
    const auto source=readTrack(cylinder,head,encoding,cell,period,attributes);writable();std::vector<FluxRevolutionData> values;std::vector<Flux::TrackFluxRevolution> revolutions;
    for(const auto& r:source.revolutions())values.push_back(replaceFluxWindow(r,start,duration,offsets,ending));if(!duration)return;
    for(const auto& r:values)revolutions.emplace_back(1,Flux::Revolution{r.indexTimeNanoseconds(),std::vector<std::uint32_t>(r.intervalsNanoseconds().begin(),r.intervalsNanoseconds().end())});
    Flux::ProtectedTrack replacement(cylinder,head,{}, {}, {}, {},std::move(revolutions));FluxTrackData result(std::move(values));const Address address{cylinder,head};
    tracks.insert_or_assign(address,std::move(replacement));cache.insert_or_assign(address,std::move(result));changed=true;
}
void RuntimeDisk::FluxState::flush(std::stop_token token){
    alive();checkCancellation(token);if(!changed)return;std::vector<Flux::ProtectedTrack> values;values.reserve(tracks.size());for(const auto& [address,track]:tracks)values.push_back(track);
    auto representation=std::make_shared<FluxRepresentation>(std::make_shared<Flux::ProtectedTrackImage>(std::move(values),false));auto options=document->mediaSource()->options();options.drive.reset();
    auto pending=std::make_shared<MediaDocument>(std::make_shared<MediaSource>(std::move(options)),document->formatId(),document->mediaKind(),representation,std::vector<M::VolumeDescriptor>{},std::vector<M::MessageRecipe>(document->messages().begin(),document->messages().end()),document->metadata(),std::vector<std::string>(document->diagnostics().begin(),document->diagnostics().end()));
    services->writing.writingService().writeAsync(pending,std::filesystem::path(QString::fromUtf8(document->mediaSource()->primaryPath()).toStdU16String()),document->formatId(),token).get();
    document=std::make_shared<MediaDocument>(document->mediaSource(),document->formatId(),document->mediaKind(),std::move(representation),std::vector<M::VolumeDescriptor>{},std::vector<M::MessageRecipe>(document->messages().begin(),document->messages().end()),document->metadata(),std::vector<std::string>(document->diagnostics().begin(),document->diagnostics().end()));changed=false;
}
}
