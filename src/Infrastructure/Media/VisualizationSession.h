#pragma once
#include "ExplorerSession.h"
#include <map>
#include <RetroMediaLab/MediaEngine/MediaVisualization.h>
#include <RetroMediaLab/MediaEngine/SectorVisualization.h>
#include <RetroMediaLab/MediaEngine/TapeSignal.h>
#include <RetroMediaLab/MediaEngine/LinearVisualization.h>
#include <RetroMediaLab/MediaEngine/SequentialVisualization.h>
#include <RetroMediaLab/MediaEngine/FluxVisualization.h>
namespace RetroMediaLab::Infrastructure {
class SimulatedDriveAudio;
struct VisualizationSnapshot {
    std::shared_ptr<const MediaEngine::MediaOpeningAnalysisResult> source;
    std::shared_ptr<const MediaEngine::MediaDocument> document;
    std::shared_ptr<const MediaEngine::VisualizationDescriptor> descriptor;
    std::shared_ptr<const MediaEngine::MediaTapeSignal> tape;
    std::optional<MediaEngine::MessageInfo> warning;
    std::optional<MediaEngine::SectorVisualizationModel> sectors;
    std::optional<MediaEngine::LinearVisualizationModel> linear;
    std::optional<MediaEngine::SequentialVisualizationModel> sequential;
    std::optional<MediaEngine::FluxVisualizationModel> flux;
};
struct SurfaceViewport {double zoom=1,panX=0,panY=0;};
class RETROMEDIALAB_INFRASTRUCTURE_API VisualizationSession final : public QObject {
public:
    explicit VisualizationSession(std::shared_ptr<ExplorerSession>);
    ~VisualizationSession() override;
    const std::shared_ptr<ExplorerSession>& opening() const noexcept { return opening_; }
    const std::shared_ptr<const VisualizationSnapshot>& snapshot() const noexcept { return snapshot_; }
    const std::optional<MediaEngine::MessageInfo>& error() const noexcept { return error_; }
    bool busy() const noexcept { return pending_.valid() || fluxPending_.valid() || presentingSegments_ || presentingTracks() || presentingLinear(); }
    bool preparing()const noexcept{return pending_.valid()||presentingSegments_||presentingTracks()||presentingLinear();}
    bool presentingSegments()const noexcept{return presentingSegments_;}
    std::size_t presentedSegments()const noexcept{return presentedSegments_;}
    const GwTrackProgress& trackProgress() const noexcept { return trackProgress_; }
    const MediaEngine::FluxVisualizationModel* displayedFlux() const noexcept;
    bool presentingLinear() const noexcept { return linearTimer_.isActive(); }
    const std::vector<GwTrackState>& linearProgress() const noexcept { return linearProgress_; }
    bool presentingTracks() const noexcept { return trackTimer_.isActive(); }
    LoadingProgress presentationProgress()const;
    void cancel();
    ExplorerSession::Subscription subscribe(std::function<void()>);
    void select(std::int32_t surface, std::int64_t position);
    void selectSector(std::uint64_t logicalBlock);
    void setZoom(double);
    void setLinkedZoom(bool);
    bool linkedZoom()const noexcept{return linkedZoom_;}
    SurfaceViewport viewport(std::int32_t)const;
    void setRevolution(std::size_t, bool synthesis = false);
    void setDiskFormat(std::optional<std::string>);
    const std::optional<std::string>& diskFormat() const noexcept { return diskFormat_; }
    const MediaEngine::SectorVisualizationModel* displayedSectors() const noexcept;
    void setPan(double x, double y);
    void inspect(std::size_t element);
    void setWaveform(bool);
    void applyClassification();
    ClassificationSelection classification;
    MediaEngine::DiskMediaCategory mediaCategory=MediaEngine::DiskMediaCategory::Unknown;
    void setFluxDecoder(std::optional<std::string>);
    std::optional<std::string> fluxDecoder;
    std::optional<MediaEngine::FluxTrackInspection> fluxInspection;
    bool fluxInspecting() const noexcept {return fluxPending_.valid();}
    bool showWaveform=false;
    bool fluxSynthesis=true;
    std::int32_t surface = 0;
    std::int64_t position = 0;
    std::size_t revolution = 0;
    double zoom = 1;
    double panX = 0, panY = 0;
    std::optional<std::uint64_t> selectedSector;
    std::optional<std::size_t> selectedElement;
    std::vector<std::uint8_t> preview;
private:
    struct PreparedTracks;
    void presentTrack();
    void presentLinear();
    void startLinear();
    void stopPresentationAudio();
    void configureFluxPresentation(const std::vector<MediaEngine::DiskTrackInfo>&);
    void startEarlyFlux(const std::shared_ptr<const MediaEngine::DiskCaptureSnapshot>&);
    void sourceChanged();
    void restoreViewport(std::int32_t);
    void saveViewport();
    bool linkedZoom_=true;
    SurfaceViewport defaultViewport_;
    std::map<std::int32_t,SurfaceViewport> viewports_;
    void inspectFlux();
    void poll();
    void publish();
    std::shared_ptr<ExplorerSession> opening_;
    ExplorerSession::Subscription openingSubscription_;
    std::vector<std::weak_ptr<const std::function<void()>>> observers_;
    std::shared_ptr<const VisualizationSnapshot> snapshot_;
    std::shared_ptr<const MediaEngine::MediaOpeningAnalysisResult> requested_;
    std::future<VisualizationSnapshot> pending_;
    std::future<std::vector<std::uint8_t>> inspection_;
    std::future<MediaEngine::FluxTrackInspection> fluxPending_;
    std::int32_t inspectingSide_=0;
    std::int64_t inspectingTrack_=0;
    std::optional<std::string> inspectingDecoder_;
    std::optional<std::size_t> inspectingRevolution_;
    std::optional<std::string> diskFormat_;
    std::optional<MediaEngine::SectorVisualizationModel> sectorPresentation_;
    std::size_t inspecting_ = 0;
    std::optional<MediaEngine::MessageInfo> error_;
    std::stop_source cancellation_;
    QTimer timer_, presentationTimer_, trackTimer_, linearTimer_;
    std::future<bool> linearRead_;
    std::vector<GwTrackState> linearProgress_;
    std::size_t nextLinear_=0;
    std::unique_ptr<SimulatedDriveAudio> driveAudio_;
    bool openingWasBusy_=false;
    std::shared_ptr<PreparedTracks> preparedTracks_;
    std::shared_ptr<const MediaEngine::DiskCaptureSnapshot> earlyCapture_;
    std::shared_ptr<std::future<MediaEngine::FluxVisualizationModel>> earlyFlux_;
    std::stop_source earlyCancellation_;
    MediaEngine::FluxVisualizationModel displayedTracks_;
    GwTrackProgress trackProgress_;
    std::size_t nextTrack_ = 0;
    std::vector<std::pair<int,int>> sectorPresentationOrder_;
    bool presentingSegments_=false;
    std::size_t presentedSegments_=0;
};
}
