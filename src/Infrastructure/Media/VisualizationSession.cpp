#include "VisualizationSession.h"
#include "SimulatedDriveAudio.h"
#include "MediaSessionConstants.h"
#include "OpeningProgressConstants.h"
#include <RetroMediaLab/MediaEngine/TapePreparation.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <chrono>
#include <deque>
#include <set>
#include <mutex>
#include <tuple>
#include <cmath>
#include <type_traits>
namespace RetroMediaLab::Infrastructure {
using namespace MediaSessionConstants;
struct VisualizationSession::PreparedTracks {
    std::mutex mutex;
    std::deque<MediaEngine::FluxVisualTrack> ready;
};

const MediaEngine::FluxVisualizationModel* VisualizationSession::displayedFlux() const noexcept {
    if (sectorPresentation_) return nullptr;
    return snapshot_ && snapshot_->flux ? &*snapshot_->flux : nullptr;
}

const MediaEngine::SectorVisualizationModel* VisualizationSession::displayedSectors() const noexcept {
    if (sectorPresentation_) return &*sectorPresentation_;
    return snapshot_ && snapshot_->sectors ? &*snapshot_->sectors : nullptr;
}

void VisualizationSession::setDiskFormat(std::optional<std::string> format) {
    if (format == diskFormat_ || preparing() || !snapshot_ || !snapshot_->flux || !snapshot_->source->diskExploration)
        return;
    if (format) {
        const auto& formats = snapshot_->source->diskExploration->info().formats;
        const auto found = std::find_if(formats.begin(), formats.end(),
            [&](const auto& value) { return value.formatId == *format; });
        if (found == formats.end() || !found->heads || !found->sectorsPerTrack || !*found->sectorsPerTrack ||
            !found->sectorSize || !*found->sectorSize) return;
        sectorPresentation_ = MediaEngine::describeSectorVisualization(*found, false, false);
        if (surface >= static_cast<std::int32_t>(sectorPresentation_->heads)) surface = 0;
        if (position >= sectorPresentation_->cylinders) position = 0;
    } else {
        sectorPresentation_.reset();
    }
    diskFormat_ = std::move(format);
    selectedSector.reset();
    fluxInspection.reset();
    inspectFlux();
    publish();
}

void VisualizationSession::presentTrack() {
    if (!preparedTracks_) {
        const auto [cylinder, head] = sectorPresentationOrder_.at(nextTrack_++);
        trackProgress_.states[{cylinder,head}] = GwTrackState::Success;
        ++trackProgress_.completedPerHead[head];
        ++trackProgress_.completed;
        driveAudio_->position({cylinder,head});
        if (nextTrack_ == sectorPresentationOrder_.size()) { trackTimer_.stop(); stopPresentationAudio(); }
        publish();
        if (trackTimer_.isActive()) trackTimer_.start();
        return;
    }
    MediaEngine::DrivePosition drivePosition;
    {
        const std::lock_guard lock(preparedTracks_->mutex);
        if (preparedTracks_->ready.empty()) return;
        auto& track = preparedTracks_->ready.front();
        trackProgress_.states[{track.cylinder, track.head}] = GwTrackState::Success;
        ++trackProgress_.completedPerHead[track.head];
        ++trackProgress_.completed;
        drivePosition = {track.cylinder, track.head};
        displayedTracks_.tracks[nextTrack_++] = std::move(track);
        preparedTracks_->ready.pop_front();
    }
    driveAudio_->position(drivePosition);
    if (nextTrack_ == displayedTracks_.tracks.size()) {
        trackTimer_.stop();
        stopPresentationAudio();
        if (!pending_.valid() && !opening_->busy()) { preparedTracks_.reset(); displayedTracks_ = {}; }
    }
    publish();
    if (trackTimer_.isActive()) trackTimer_.start();
}

VisualizationSession::VisualizationSession(std::shared_ptr<ExplorerSession> opening) : opening_(std::move(opening)), driveAudio_(std::make_unique<SimulatedDriveAudio>()) {
    trackTimer_.setTimerType(Qt::PreciseTimer);
    connect(&trackTimer_, &QTimer::timeout, this, [this] { presentTrack(); });
    timer_.setInterval(PollMilliseconds);
    connect(&timer_, &QTimer::timeout, this, [this] { poll(); });
    openingSubscription_ = opening_->subscribe([this] { sourceChanged(); });
    presentationTimer_.setInterval(TapePresentationMilliseconds);
    connect(&presentationTimer_, &QTimer::timeout, this, [this] {
        ++presentedSegments_;
        if (presentedSegments_ >= snapshot_->sequential->segments.size()) {
            presentationTimer_.stop();
            presentingSegments_ = false;
            stopPresentationAudio();
        }
        publish();
    });
    linearTimer_.setTimerType(Qt::PreciseTimer);
    connect(&linearTimer_, &QTimer::timeout, this, [this] { presentLinear(); });
    sourceChanged();
}
VisualizationSession::~VisualizationSession() {
    earlyCancellation_.request_stop();
    linearTimer_.stop();
    driveAudio_->cancel();
    trackTimer_.stop();presentationTimer_.stop();timer_.stop(); openingSubscription_.reset(); cancellation_.request_stop();
    if (pending_.valid()) pending_.wait();
    if (linearRead_.valid()) linearRead_.wait();
    if (inspection_.valid()) inspection_.wait();
    if (fluxPending_.valid()) fluxPending_.wait();
}
void VisualizationSession::cancel(){earlyCancellation_.request_stop();opening_->cancel();cancellation_.request_stop();trackTimer_.stop();preparedTracks_.reset();displayedTracks_={};trackProgress_={};presentationTimer_.stop();presentingSegments_=false;linearTimer_.stop();driveAudio_->cancel();publish();}
LoadingProgress VisualizationSession::presentationProgress()const{LoadingProgress value;value.formatCheck=opening_->progress().formatCheck;value.stage.resourceKey=presentingSegments_?OpeningProgressConstants::TapeReading:OpeningProgressConstants::Display;value.detail=opening_->path().toStdString();if(presentingSegments_&&snapshot_&&snapshot_->sequential){const auto total=snapshot_->sequential->segments.size();value.detailMessage=MediaEngine::MessageInfo{OpeningProgressConstants::TapeRecord,{std::to_string(presentedSegments_),std::to_string(total)},{}};value.value=OpeningProgressConstants::Maximum*presentedSegments_/std::max<std::size_t>(1,total);value.indeterminate=false;}
    else if (presentingLinear()) {
        value.value = OpeningProgressConstants::Maximum * nextLinear_ / linearProgress_.size();
        value.indeterminate = false;
    }
    else if (presentingTracks() && !trackProgress_.cylinders.empty()) {
        namespace C = OpeningProgressConstants;
        const auto total = trackProgress_.cylinders.size();
        value.detailMessage = MediaEngine::MessageInfo{C::Track,
            {std::to_string(trackProgress_.completed),std::to_string(total)},{}};
        value.value = C::Maximum * trackProgress_.completed / total;
        value.indeterminate = false;
    }
    return value;
}
void VisualizationSession::startLinear() {
    linearProgress_.assign(snapshot_->descriptor->fields().elements.size(), GwTrackState::Pending);
    nextLinear_ = 0;
    if (linearProgress_.empty()) return;

    const auto kind = snapshot_->linear->cartridge ? MediaEngine::MediaKind::Cartridge :
        snapshot_->descriptor->fields().representationKind == MediaEngine::RepresentationKind::OpticalTracks
            ? MediaEngine::MediaKind::Optical : MediaEngine::MediaKind::HardDisk;
    driveAudio_->start(kind);
    const auto interval = std::clamp<std::size_t>(LinearPresentationBudgetMilliseconds / linearProgress_.size(),
        MinimumLinearPresentationMilliseconds, TrackPresentationMilliseconds);
    linearTimer_.start(static_cast<int>(interval));
}

void VisualizationSession::presentLinear() {
    const auto& elements = snapshot_->descriptor->fields().elements;
    if (linearRead_.valid()) {
        if (linearRead_.wait_for(std::chrono::seconds::zero()) != std::future_status::ready) return;
        try {
            linearProgress_[nextLinear_] = linearRead_.get() ? GwTrackState::Success : GwTrackState::Failed;
        } catch (const std::exception&) {
            linearProgress_[nextLinear_] = GwTrackState::Failed;
        }
        ++nextLinear_;
        if (nextLinear_ == elements.size()) {
            linearTimer_.stop();
            stopPresentationAudio();
            publish();
            return;
        }
        publish();
        if (!linearTimer_.isActive()) return;
    }

    const auto& element = elements[nextLinear_];
    const auto& fields = element.fields();
    const auto head = fields.surface.value_or(0);
    auto cylinder = fields.trackNumber.value_or(0);
    if (const auto& geometry = snapshot_->linear->geometry)
        cylinder = static_cast<std::int32_t>(fields.position / geometry->sectorsPerTrack() / geometry->heads());
    driveAudio_->position({cylinder, head, -1, fields.position});
    try {
        linearRead_ = MediaEngine::checkLinearVisualizationElementAsync(snapshot_->document, element, cancellation_.get_token());
    } catch (const std::exception& error) {
        linearTimer_.stop();
        stopPresentationAudio();
        error_ = MediaEngine::MessageInfo{UnexpectedError,{error.what()},{}};
        publish();
    }
}

void VisualizationSession::stopPresentationAudio() {
    if (!presentingTracks() && !presentingSegments() && !presentingLinear()) driveAudio_->finish();
}

void VisualizationSession::configureFluxPresentation(const std::vector<MediaEngine::DiskTrackInfo>& tracks) {
    displayedTracks_ = {};
    trackProgress_ = {};
    nextTrack_ = 0;
    for (const auto& track : tracks) {
        MediaEngine::FluxVisualTrack placeholder;
        placeholder.head = track.head;
        placeholder.cylinder = track.cylinder;
        displayedTracks_.tracks.push_back(std::move(placeholder));
        trackProgress_.cylindersPerHead[track.head].push_back(track.cylinder);
    }
    std::stable_sort(displayedTracks_.tracks.begin(), displayedTracks_.tracks.end(),
        [](const auto& a, const auto& b) {
            return std::tie(a.cylinder,a.head) < std::tie(b.cylinder,b.head);
        });
    for (auto& [head, cylinders] : trackProgress_.cylindersPerHead) {
        std::sort(cylinders.begin(), cylinders.end());
        trackProgress_.heads.push_back(head);
        trackProgress_.cylinders.insert(trackProgress_.cylinders.end(), cylinders.begin(), cylinders.end());
    }
    preparedTracks_ = std::make_shared<PreparedTracks>();
    driveAudio_->start(MediaEngine::MediaKind::Floppy);
    trackTimer_.start(ScpTrackPresentationMilliseconds);
}

void VisualizationSession::startEarlyFlux(const std::shared_ptr<const MediaEngine::DiskCaptureSnapshot>& capture) {
    if (!capture || capture->tracks.empty() || earlyCapture_ == capture || pending_.valid()) return;
    earlyCancellation_.request_stop();
    earlyCapture_ = capture;
    earlyCancellation_ = std::stop_source{};
    const auto token = earlyCancellation_.get_token();
    configureFluxPresentation(capture->tracks);
    const auto prepared = preparedTracks_;
    try {
        earlyFlux_ = std::make_shared<std::future<MediaEngine::FluxVisualizationModel>>(
            std::async(std::launch::async,[capture,prepared,token] {
                MediaEngine::DiskImageInfo disk;
                disk.tracks = capture->tracks;
                return MediaEngine::describeFluxVisualization(disk,token,[prepared](const auto& track) {
                    const std::lock_guard lock(prepared->mutex);
                    prepared->ready.push_back(track);
                });
            }));
    } catch (...) {
        trackTimer_.stop();
        preparedTracks_.reset();
        displayedTracks_ = {};
        trackProgress_ = {};
    }
    publish();
}

void VisualizationSession::sourceChanged() {
    if (opening_->busy()) {
        if (!openingWasBusy_) {
            openingWasBusy_ = true;
            diskFormat_.reset(); sectorPresentation_.reset(); fluxSynthesis = true; revolution = 0;
            cancellation_.request_stop();
            linearTimer_.stop();
            linearProgress_.clear();
            presentationTimer_.stop();
            presentingSegments_ = false;
            trackTimer_.stop();
            preparedTracks_.reset();
            displayedTracks_ = {};
            trackProgress_ = {};
            driveAudio_->cancel();
        }
        startEarlyFlux(opening_->progress().capture);
        return;
    }
    openingWasBusy_ = false;
    if (!opening_->result() || opening_->result() == requested_) {
        if (earlyFlux_) {
            earlyCancellation_.request_stop();
            earlyFlux_.reset();
            trackTimer_.stop();
            driveAudio_->cancel();
            preparedTracks_.reset();
            displayedTracks_ = {};
            trackProgress_ = {};
            publish();
        }
        return;
    }
    cancellation_.request_stop();presentationTimer_.stop();presentingSegments_=false;
    if (linearRead_.valid()) { timer_.start(); return; }
    if (pending_.valid() || inspection_.valid() || fluxPending_.valid()) { timer_.start(); return; }
    if (!earlyFlux_) {
        trackTimer_.stop();
        preparedTracks_.reset();
        displayedTracks_ = {};
        trackProgress_ = {};
        nextTrack_ = 0;
    }
    sectorPresentationOrder_.clear();
    requested_ = opening_->result();
    cancellation_ = std::stop_source{};
    error_.reset();
    const auto token = cancellation_.get_token();
    const auto source = requested_;
    const auto path = std::filesystem::path(opening_->path().toStdU16String());
    if (source->diskExploration) {
        classification.detect(source->diskExploration->info());
        const auto selected = classification.visualization();
        mediaCategory = selected.mediaCategory;
        fluxDecoder = selected.decoderId;
    }
    if (!earlyFlux_ && source->diskExploration &&
        std::any_of(source->diskExploration->info().tracks.begin(),source->diskExploration->info().tracks.end(),
            [](const auto& track) { return !track.revolutions.empty(); }))
        configureFluxPresentation(source->diskExploration->info().tracks);
    const auto prepared = preparedTracks_;
    auto early = std::move(earlyFlux_);
    try { pending_ = std::async(std::launch::async, [source, token, path, prepared, early] {
        VisualizationSnapshot value;
        value.source = source;
        value.document = MediaEngine::prepareMediaVisualization(source->document, token);
        value.descriptor = std::make_shared<const MediaEngine::VisualizationDescriptor>(MediaEngine::describeMediaVisualization(value.document, token));
        if(value.descriptor->fields().representationKind==MediaEngine::RepresentationKind::Flux && source->diskExploration)value.flux=early ? early->get() : MediaEngine::describeFluxVisualization(source->diskExploration->info(), token,
            [prepared](const auto& track) {
                if (!prepared) return;
                const std::lock_guard lock(prepared->mutex);
                prepared->ready.push_back(track);
            });
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
    catch (const std::exception& error) { trackTimer_.stop();preparedTracks_.reset();displayedTracks_={};trackProgress_={};error_=MediaEngine::MessageInfo{UnexpectedError,{error.what()},{}}; }
    catch (...) { trackTimer_.stop();preparedTracks_.reset();displayedTracks_={};trackProgress_={};error_=MediaEngine::MessageInfo{UnexpectedError,{},{}}; }
    publish();
}
void VisualizationSession::poll() {
    const auto captureError = [this](const auto& error) {
        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(error)>, MediaEngine::MediaError>)
            error_ = MediaEngine::MessageInfo{std::string(MediaEngine::diagnosticKey(error.diagnostic().code)), error.diagnostic().arguments, MediaDomain};
        else error_ = MediaEngine::MessageInfo{UnexpectedError, {error.what()}, {}};
    };
    bool changed = false;
    if (!linearTimer_.isActive() && linearRead_.valid() &&
        linearRead_.wait_for(std::chrono::seconds::zero()) == std::future_status::ready) {
        try { static_cast<void>(linearRead_.get()); } catch (...) {}
    }
    if (pending_.valid() && pending_.wait_for(std::chrono::seconds::zero()) == std::future_status::ready) {
        try {
            auto result = pending_.get();
            if (!cancellation_.stop_requested()) {
                snapshot_ = std::make_shared<const VisualizationSnapshot>(std::move(result));
                linearProgress_.clear();
                if (snapshot_->linear) startLinear();
                presentedSegments_ = 0;
                presentingSegments_ = snapshot_->sequential && !snapshot_->sequential->segments.empty();
                if (presentingSegments_) {
                    driveAudio_->start(MediaEngine::MediaKind::Tape);
                    presentationTimer_.start();
                }
                showWaveform=snapshot_->sequential && snapshot_->sequential->segments.empty() && snapshot_->tape;
                fluxInspection.reset();
                viewports_.clear();defaultViewport_={};
                selectedElement.reset(); preview.clear(); surface = 0; position = 0; revolution = 0; zoom = 1; panX = 0; panY = 0; selectedSector.reset();
                if(snapshot_->flux && !snapshot_->flux->tracks.empty()){surface=snapshot_->flux->tracks.front().head;position=snapshot_->flux->tracks.front().cylinder;inspectFlux();}
            }
        } catch (const MediaEngine::MediaError& error) { if (!cancellation_.stop_requested()) captureError(error); }
          catch (const std::exception& error) { if (!cancellation_.stop_requested()) captureError(error); }
          catch (...) { if (!cancellation_.stop_requested()) error_ = MediaEngine::MessageInfo{UnexpectedError, {}, {}}; }
        if (!snapshot_ || snapshot_->source != requested_ || !snapshot_->flux) {
            trackTimer_.stop(); preparedTracks_.reset(); displayedTracks_ = {}; trackProgress_ = {};
        } else if (!trackTimer_.isActive()) {
            preparedTracks_.reset(); displayedTracks_ = {};
        }
        if (!cancellation_.stop_requested() && snapshot_ && snapshot_->source == requested_ &&
            snapshot_->sectors && !snapshot_->sectors->logical) {
            std::set<std::pair<int,int>> positions;
            for (const auto& sector : snapshot_->sectors->sectors)
                positions.emplace(sector.cylinder,sector.head);
            sectorPresentationOrder_.assign(positions.begin(),positions.end());
            for (const auto& [cylinder,head] : sectorPresentationOrder_) {
                trackProgress_.cylindersPerHead[head].push_back(cylinder);
                trackProgress_.cylinders.push_back(cylinder);
            }
            for (const auto& [head,cylinders] : trackProgress_.cylindersPerHead)
                trackProgress_.heads.push_back(head);
            if (!sectorPresentationOrder_.empty()) {
                driveAudio_->start(MediaEngine::MediaKind::Floppy);
                trackTimer_.start(SectorTrackPresentationMilliseconds);
            }
        }
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
        const bool current=inspectingSide_==surface && inspectingTrack_==position && inspectingDecoder_==fluxDecoder && !sectorPresentation_ &&
            inspectingRevolution_==(fluxSynthesis?std::nullopt:std::optional{revolution});
        try{auto result=fluxPending_.get();if(!cancellation_.stop_requested() && current)fluxInspection=std::move(result);}
        catch(const MediaEngine::MediaError& error){if(!cancellation_.stop_requested() && current)captureError(error);}
        catch(const std::exception& error){if(!cancellation_.stop_requested() && current)captureError(error);}
        catch(...){if(!cancellation_.stop_requested() && current)error_=MediaEngine::MessageInfo{UnexpectedError,{},{}};}
        if(!cancellation_.stop_requested() && !current)inspectFlux();changed=true;
    }
    if (!pending_.valid() && !inspection_.valid() && !fluxPending_.valid()) { timer_.stop(); sourceChanged(); }
    if (changed) publish();
}
void VisualizationSession::select(std::int32_t side, std::int64_t track) {if(surface==side && position==track)return;selectedSector.reset();fluxInspection.reset();restoreViewport(side);surface=side;position=track;inspectFlux();publish();}
void VisualizationSession::applyClassification(){const auto value=classification.visualization();mediaCategory=value.mediaCategory;setFluxDecoder(value.decoderId);publish();}
void VisualizationSession::setFluxDecoder(std::optional<std::string> decoder){if(fluxDecoder==decoder)return;fluxDecoder=std::move(decoder);fluxInspection.reset();selectedSector.reset();inspectFlux();publish();}
void VisualizationSession::inspectFlux(){
    if(sectorPresentation_ || !snapshot_ || !snapshot_->flux || !snapshot_->source->diskExploration || fluxPending_.valid() || cancellation_.stop_requested())return;
    const auto source=snapshot_->source;const auto& tracks=source->diskExploration->info().tracks;
    const auto found=std::find_if(tracks.begin(),tracks.end(),[this](const auto& track){return track.head==surface && track.cylinder==position;});if(found==tracks.end())return;
    const auto index=static_cast<std::size_t>(found-tracks.begin());inspectingSide_=surface;inspectingTrack_=position;inspectingDecoder_=fluxDecoder;const auto token=cancellation_.get_token();const auto decoder=fluxDecoder;const auto turn=fluxSynthesis?std::nullopt:std::optional{revolution};inspectingRevolution_=turn;error_.reset();
    try{fluxPending_=std::async(std::launch::async,[source,index,decoder,token,turn]{return MediaEngine::inspectFluxVisualization(source->diskExploration->info().tracks[index],decoder,token,turn);});timer_.start();}catch(const std::exception& error){error_=MediaEngine::MessageInfo{UnexpectedError,{error.what()},{}};}
}
void VisualizationSession::selectSector(std::uint64_t logical){const auto* model=displayedSectors();if(!model)return;const auto& sectors=model->sectors;const auto found=std::find_if(sectors.begin(),sectors.end(),[&](const auto& item){return item.logicalBlock==logical;});if(found==sectors.end())return;if(surface==found->head && position==found->cylinder && selectedSector==logical)return;restoreViewport(static_cast<std::int32_t>(found->head));surface=static_cast<std::int32_t>(found->head);position=found->cylinder;selectedSector=logical;publish();}
SurfaceViewport VisualizationSession::viewport(std::int32_t head)const{if(linkedZoom_)return {zoom,panX,panY};const auto found=viewports_.find(head);return found==viewports_.end()?defaultViewport_:found->second;}
void VisualizationSession::saveViewport(){if(!linkedZoom_)viewports_[surface]={zoom,panX,panY};}
void VisualizationSession::restoreViewport(std::int32_t head){if(head==surface||linkedZoom_)return;const auto value=viewport(head);zoom=value.zoom;panX=value.panX;panY=value.panY;}
void VisualizationSession::setLinkedZoom(bool value){if(value==linkedZoom_)return;defaultViewport_={zoom,panX,panY};viewports_.clear();linkedZoom_=value;publish();}
void VisualizationSession::setZoom(double value) { if (std::isfinite(value) && value >= 1 && value <= MaximumZoom) { zoom = value; if (value == 1) { panX = 0; panY = 0; } saveViewport();publish(); } }
void VisualizationSession::setRevolution(std::size_t value, bool synthesis) {
    if (revolution == value && fluxSynthesis == synthesis) return;
    revolution = value;
    fluxSynthesis = synthesis;
    selectedSector.reset();
    fluxInspection.reset();
    inspectFlux();
    publish();
}
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
