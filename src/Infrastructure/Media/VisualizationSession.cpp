#include "VisualizationSession.h"
#include "MediaSessionConstants.h"
#include <RetroMediaLab/MediaEngine/TapePreparation.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <type_traits>
namespace RetroMediaLab::Infrastructure {
using namespace MediaSessionConstants;
VisualizationSession::VisualizationSession(std::shared_ptr<ExplorerSession> opening) : opening_(std::move(opening)) {
    timer_.setInterval(PollMilliseconds);
    connect(&timer_, &QTimer::timeout, this, [this] { poll(); });
    openingSubscription_ = opening_->subscribe([this] { sourceChanged(); });
    sourceChanged();
}
VisualizationSession::~VisualizationSession() {
    timer_.stop(); openingSubscription_.reset(); cancellation_.request_stop();
    if (pending_.valid()) pending_.wait();
    if (inspection_.valid()) inspection_.wait();
    if (fluxPending_.valid()) fluxPending_.wait();
}
void VisualizationSession::sourceChanged() {
    if (!opening_->result() || opening_->result() == requested_) return;
    cancellation_.request_stop();
    if (pending_.valid() || inspection_.valid() || fluxPending_.valid()) { timer_.start(); return; }
    requested_ = opening_->result();
    cancellation_ = std::stop_source{};
    error_.reset();
    const auto token = cancellation_.get_token();
    const auto source = requested_;
    const auto path = std::filesystem::path(opening_->path().toStdU16String());
    try { pending_ = std::async(std::launch::async, [source, token, path] {
        VisualizationSnapshot value;
        value.source = source;
        value.document = MediaEngine::prepareMediaVisualization(source->document, token);
        value.descriptor = std::make_shared<const MediaEngine::VisualizationDescriptor>(MediaEngine::describeMediaVisualization(value.document, token));
        if(value.descriptor->fields().representationKind==MediaEngine::RepresentationKind::Flux && source->diskExploration)value.flux=MediaEngine::describeFluxVisualization(source->diskExploration->info(),token);
        if(value.descriptor->fields().representationKind==MediaEngine::RepresentationKind::Sectors && source->diskExploration){
            const auto& disk=source->diskExploration->info();const auto found=std::find_if(disk.formats.begin(),disk.formats.end(),[&](const auto& format){return format.formatId==disk.primaryFormatId;});
            if(found!=disk.formats.end())value.sectors=MediaEngine::describeSectorVisualization(value.document,*found);
            else if(!disk.formats.empty())value.sectors=MediaEngine::describeSectorVisualization(value.document,disk.formats.front());
        }
        if(value.descriptor->fields().representationKind==MediaEngine::RepresentationKind::Blocks || value.descriptor->fields().representationKind==MediaEngine::RepresentationKind::OpticalTracks)value.linear=MediaEngine::describeLinearVisualization(value.document,*value.descriptor,token);
        if (value.descriptor->fields().representationKind == MediaEngine::RepresentationKind::Sequential) {
            value.sequential=MediaEngine::describeSequentialVisualization(value.document,*value.descriptor,token);
            try { value.tape = std::make_shared<const MediaEngine::MediaTapeSignal>(MediaEngine::readTapeAsync(path, token).get()); }
            catch (const MediaEngine::MediaError& error) { MediaEngine::checkCancellation(token); value.warning=MediaEngine::MessageInfo{std::string(MediaEngine::diagnosticKey(error.diagnostic().code)),error.diagnostic().arguments,MediaDomain}; }
        }
        return value;
    }); timer_.start(); }
    catch (const std::exception& error) { error_=MediaEngine::MessageInfo{UnexpectedError,{error.what()},{}}; }
    catch (...) { error_=MediaEngine::MessageInfo{UnexpectedError,{},{}}; }
    publish();
}
void VisualizationSession::poll() {
    const auto captureError = [this](const auto& error) {
        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(error)>, MediaEngine::MediaError>)
            error_ = MediaEngine::MessageInfo{std::string(MediaEngine::diagnosticKey(error.diagnostic().code)), error.diagnostic().arguments, MediaDomain};
        else error_ = MediaEngine::MessageInfo{UnexpectedError, {error.what()}, {}};
    };
    bool changed = false;
    if (pending_.valid() && pending_.wait_for(std::chrono::seconds::zero()) == std::future_status::ready) {
        try {
            auto result = pending_.get();
            if (!cancellation_.stop_requested()) {
                snapshot_ = std::make_shared<const VisualizationSnapshot>(std::move(result));
                showWaveform=snapshot_->sequential && snapshot_->sequential->segments.empty() && snapshot_->tape;
                fluxInspection.reset();
                viewports_.clear();defaultViewport_={};
                selectedElement.reset(); preview.clear(); surface = 0; position = 0; revolution = 0; zoom = 1; panX = 0; panY = 0; selectedSector.reset();
                if(snapshot_->flux && !snapshot_->flux->tracks.empty()){surface=snapshot_->flux->tracks.front().head;position=snapshot_->flux->tracks.front().cylinder;inspectFlux();}
            }
        } catch (const MediaEngine::MediaError& error) { if (!cancellation_.stop_requested()) captureError(error); }
          catch (const std::exception& error) { if (!cancellation_.stop_requested()) captureError(error); }
          catch (...) { if (!cancellation_.stop_requested()) error_ = MediaEngine::MessageInfo{UnexpectedError, {}, {}}; }
        changed = true;
    }
    if (inspection_.valid() && inspection_.wait_for(std::chrono::seconds::zero()) == std::future_status::ready) {
        try { auto result = inspection_.get(); if (!cancellation_.stop_requested() && selectedElement==inspecting_) preview=std::move(result); }
        catch (const MediaEngine::MediaError& error) { if (!cancellation_.stop_requested()) captureError(error); }
        catch (const std::exception& error) { if (!cancellation_.stop_requested()) captureError(error); }
        catch (...) { if (!cancellation_.stop_requested()) error_ = MediaEngine::MessageInfo{UnexpectedError, {}, {}}; }
        if(!cancellation_.stop_requested() && selectedElement && *selectedElement!=inspecting_)inspect(*selectedElement);
        changed = true;
    }
    if(fluxPending_.valid() && fluxPending_.wait_for(std::chrono::seconds::zero())==std::future_status::ready){
        const bool current=inspectingSide_==surface && inspectingTrack_==position && inspectingDecoder_==fluxDecoder;
        try{auto result=fluxPending_.get();if(!cancellation_.stop_requested() && current)fluxInspection=std::move(result);}
        catch(const MediaEngine::MediaError& error){if(!cancellation_.stop_requested() && current)captureError(error);}
        catch(const std::exception& error){if(!cancellation_.stop_requested() && current)captureError(error);}
        catch(...){if(!cancellation_.stop_requested() && current)error_=MediaEngine::MessageInfo{UnexpectedError,{},{}};}
        if(!cancellation_.stop_requested() && !current)inspectFlux();changed=true;
    }
    if (!pending_.valid() && !inspection_.valid() && !fluxPending_.valid()) { timer_.stop(); sourceChanged(); }
    if (changed) publish();
}
void VisualizationSession::select(std::int32_t side, std::int64_t track) {if(surface==side && position==track)return;selectedSector.reset();fluxInspection.reset();restoreViewport(side);surface=side;position=track;revolution=0;inspectFlux();publish();}
void VisualizationSession::setFluxDecoder(std::optional<std::string> decoder){if(fluxDecoder==decoder)return;fluxDecoder=std::move(decoder);fluxInspection.reset();selectedSector.reset();inspectFlux();publish();}
void VisualizationSession::inspectFlux(){
    if(!snapshot_ || !snapshot_->flux || !snapshot_->source->diskExploration || fluxPending_.valid() || cancellation_.stop_requested())return;
    const auto source=snapshot_->source;const auto& tracks=source->diskExploration->info().tracks;
    const auto found=std::find_if(tracks.begin(),tracks.end(),[this](const auto& track){return track.head==surface && track.cylinder==position;});if(found==tracks.end())return;
    const auto index=static_cast<std::size_t>(found-tracks.begin());inspectingSide_=surface;inspectingTrack_=position;inspectingDecoder_=fluxDecoder;const auto token=cancellation_.get_token();const auto decoder=fluxDecoder;error_.reset();
    try{fluxPending_=std::async(std::launch::async,[source,index,decoder,token]{return MediaEngine::inspectFluxVisualization(source->diskExploration->info().tracks[index],decoder,token);});timer_.start();}catch(const std::exception& error){error_=MediaEngine::MessageInfo{UnexpectedError,{error.what()},{}};}
}
void VisualizationSession::selectSector(std::uint64_t logical){if(!snapshot_ || !snapshot_->sectors)return;const auto& sectors=snapshot_->sectors->sectors;const auto found=std::find_if(sectors.begin(),sectors.end(),[&](const auto& item){return item.logicalBlock==logical;});if(found==sectors.end())return;if(surface==found->head && position==found->cylinder && selectedSector==logical)return;restoreViewport(static_cast<std::int32_t>(found->head));surface=static_cast<std::int32_t>(found->head);position=found->cylinder;selectedSector=logical;publish();}
SurfaceViewport VisualizationSession::viewport(std::int32_t head)const{if(linkedZoom_)return {zoom,panX,panY};const auto found=viewports_.find(head);return found==viewports_.end()?defaultViewport_:found->second;}
void VisualizationSession::saveViewport(){if(!linkedZoom_)viewports_[surface]={zoom,panX,panY};}
void VisualizationSession::restoreViewport(std::int32_t head){if(head==surface||linkedZoom_)return;const auto value=viewport(head);zoom=value.zoom;panX=value.panX;panY=value.panY;}
void VisualizationSession::setLinkedZoom(bool value){if(value==linkedZoom_)return;defaultViewport_={zoom,panX,panY};viewports_.clear();linkedZoom_=value;publish();}
void VisualizationSession::setZoom(double value) { if (std::isfinite(value) && value >= 1 && value <= MaximumZoom) { zoom = value; if (value == 1) { panX = 0; panY = 0; } saveViewport();publish(); } }
void VisualizationSession::setRevolution(std::size_t value) { revolution = value; publish(); }
void VisualizationSession::setPan(double x, double y) { if (std::isfinite(x) && std::isfinite(y)) { panX = x; panY = y; saveViewport();publish(); } }
void VisualizationSession::setWaveform(bool value){if(value==showWaveform)return;showWaveform=value;zoom=1;panX=0;panY=0;publish();}
void VisualizationSession::inspect(std::size_t element) {
    if (!snapshot_ || (!snapshot_->linear && !snapshot_->sequential) || pending_.valid() || element >= snapshot_->descriptor->fields().elements.size()) return;
    if(selectedElement!=element){selectedElement=element;preview.clear();error_.reset();}
    if(inspection_.valid()){publish();return;}
    inspecting_ = element;
    inspection_ = snapshot_->sequential?MediaEngine::readSequentialVisualizationPreviewAsync(snapshot_->document,element,cancellation_.get_token()):MediaEngine::readVisualizationPreviewAsync(snapshot_->document, snapshot_->descriptor->fields().elements[element], cancellation_.get_token());
    timer_.start();publish();
}
ExplorerSession::Subscription VisualizationSession::subscribe(std::function<void()> callback) {
    auto value = std::make_shared<const std::function<void()>>(std::move(callback)); observers_.push_back(value); return value;
}
void VisualizationSession::publish() {
    const auto observers = observers_;
    std::erase_if(observers_, [](const auto& value) { return value.expired(); });
    for (const auto& observer : observers) if (const auto callback = observer.lock()) (*callback)();
}
}
