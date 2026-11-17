#pragma once
#include "ExplorerSession.h"
#include <RetroMediaLab/MediaEngine/MediaVisualization.h>
#include <RetroMediaLab/MediaEngine/SectorVisualization.h>
#include <RetroMediaLab/MediaEngine/TapeSignal.h>
#include <RetroMediaLab/MediaEngine/LinearVisualization.h>
#include <RetroMediaLab/MediaEngine/SequentialVisualization.h>
namespace RetroMediaLab::Infrastructure {
struct VisualizationSnapshot {
    std::shared_ptr<const MediaEngine::MediaOpeningAnalysisResult> source;
    std::shared_ptr<const MediaEngine::MediaDocument> document;
    std::shared_ptr<const MediaEngine::VisualizationDescriptor> descriptor;
    std::shared_ptr<const MediaEngine::MediaTapeSignal> tape;
    std::optional<MediaEngine::MessageInfo> warning;
    std::optional<MediaEngine::SectorVisualizationModel> sectors;
    std::optional<MediaEngine::LinearVisualizationModel> linear;
    std::optional<MediaEngine::SequentialVisualizationModel> sequential;
};
class RETROMEDIALAB_INFRASTRUCTURE_API VisualizationSession final : public QObject {
public:
    explicit VisualizationSession(std::shared_ptr<ExplorerSession>);
    ~VisualizationSession() override;
    const std::shared_ptr<ExplorerSession>& opening() const noexcept { return opening_; }
    const std::shared_ptr<const VisualizationSnapshot>& snapshot() const noexcept { return snapshot_; }
    const std::optional<MediaEngine::MessageInfo>& error() const noexcept { return error_; }
    bool busy() const noexcept { return pending_.valid(); }
    ExplorerSession::Subscription subscribe(std::function<void()>);
    void select(std::int32_t surface, std::int64_t position);
    void selectSector(std::uint64_t logicalBlock);
    void setZoom(double);
    void setRevolution(std::size_t);
    void setPan(double x, double y);
    void inspect(std::size_t element);
    void setWaveform(bool);
    bool showWaveform=false;
    std::int32_t surface = 0;
    std::int64_t position = 0;
    std::size_t revolution = 0;
    double zoom = 1;
    double panX = 0, panY = 0;
    std::optional<std::uint64_t> selectedSector;
    std::optional<std::size_t> selectedElement;
    std::vector<std::uint8_t> preview;
private:
    void sourceChanged();
    void poll();
    void publish();
    std::shared_ptr<ExplorerSession> opening_;
    ExplorerSession::Subscription openingSubscription_;
    std::vector<std::weak_ptr<const std::function<void()>>> observers_;
    std::shared_ptr<const VisualizationSnapshot> snapshot_;
    std::shared_ptr<const MediaEngine::MediaOpeningAnalysisResult> requested_;
    std::future<VisualizationSnapshot> pending_;
    std::future<std::vector<std::uint8_t>> inspection_;
    std::size_t inspecting_ = 0;
    std::optional<MediaEngine::MessageInfo> error_;
    std::stop_source cancellation_;
    QTimer timer_;
};
}
