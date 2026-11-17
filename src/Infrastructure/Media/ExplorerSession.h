#pragma once
#include "InfrastructureApi.h"
#include <RetroMediaLab/MediaEngine/MediaExploration.h>
#include <QObject>
#include <QTimer>
#include <QString>
#include <functional>
#include <memory>
#include <stop_token>
#include <vector>

namespace RetroMediaLab::Infrastructure {
// Owns the operation and its result independently of any theme widget.
class RETROMEDIALAB_INFRASTRUCTURE_API ExplorerSession final : public QObject {
public:
    using Subscription = std::shared_ptr<const std::function<void()>>;
    ExplorerSession();
    ~ExplorerSession() override;
    void open(const QString& path, std::optional<std::string> format = {}, std::shared_ptr<const void> sourceLifetime = {});
    void cancel();
    bool busy() const noexcept { return pending_.valid(); }
    const QString& path() const noexcept { return path_; }
    const QString& error() const noexcept { return error_; }
    const std::optional<MediaEngine::MessageInfo>& errorMessage() const noexcept { return errorMessage_; }
    const std::shared_ptr<const MediaEngine::MediaOpeningAnalysisResult>& result() const noexcept { return result_; }
    Subscription subscribe(std::function<void()>);
    std::size_t selectedVolume = 0;
    std::vector<std::size_t> folderPath;
    std::optional<std::size_t> selectedEntry;
    std::optional<std::int32_t> selectedSession;
    std::optional<std::string> selectedFormat;
private:
    void poll();
    void notify();
    QTimer timer_;
    std::stop_source cancellation_;
    std::future<MediaEngine::MediaOpeningAnalysisResult> pending_;
    std::shared_ptr<const MediaEngine::MediaOpeningAnalysisResult> result_;
    QString path_, pendingPath_, error_;
    std::optional<std::string> pendingFormat_;
    std::optional<MediaEngine::MessageInfo> errorMessage_;
    std::shared_ptr<const void> pendingLifetime_;
    bool cancelled_ = false;
    std::vector<std::weak_ptr<const std::function<void()>>> observers_;
};
}
