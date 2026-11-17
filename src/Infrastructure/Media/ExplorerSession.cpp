#include "ExplorerSession.h"
#include "MediaSessionConstants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <chrono>
#include <algorithm>
#include <exception>

namespace RetroMediaLab::Infrastructure {
using namespace MediaSessionConstants;
ExplorerSession::ExplorerSession() {
    timer_.setInterval(PollMilliseconds);
    connect(&timer_, &QTimer::timeout, this, [this] { poll(); });
}
ExplorerSession::~ExplorerSession() {
    timer_.stop();
    cancellation_.request_stop();
    // The asynchronous operation must finish before its resources are destroyed.
    if (pending_.valid()) pending_.wait();
}
void ExplorerSession::open(const QString& path, std::optional<std::string> format) {
    if (busy() || path.isEmpty()) return;
    cancellation_ = std::stop_source{};
    cancelled_ = false;
    error_.clear();
    errorMessage_.reset();
    pendingPath_ = path;
    pendingFormat_ = format;
    try {
        pending_ = MediaEngine::analyzeMediaOpeningAsync(std::filesystem::path(path.toStdU16String()), std::move(format), {}, {}, cancellation_.get_token());
        timer_.start();
    } catch (const MediaEngine::MediaError& error) {
        error_ = QString::fromUtf8(error.what());
        errorMessage_ = MediaEngine::MessageInfo{std::string(MediaEngine::diagnosticKey(error.diagnostic().code)), error.diagnostic().arguments, MediaDomain};
    } catch (const std::exception& error) {
        error_ = QString::fromUtf8(error.what());
    } catch (...) {
        error_ = QString::fromLatin1(UnexpectedError);
    }
    notify();
}
void ExplorerSession::cancel() {
    if (!busy()) return;
    cancelled_ = true;
    cancellation_.request_stop();
}
void ExplorerSession::poll() {
    if (!pending_.valid() || pending_.wait_for(std::chrono::seconds::zero()) != std::future_status::ready) return;
    timer_.stop();
    try {
        auto value = pending_.get();
        if (!cancelled_) {
            result_ = std::make_shared<const MediaEngine::MediaOpeningAnalysisResult>(std::move(value));
            path_ = pendingPath_;
            selectedFormat = pendingFormat_;
            selectedVolume = 0;
            selectedSession.reset();
            if (result_->mediaExploration) {
                const auto& volumes = result_->mediaExploration->volumes;
                for (std::size_t index = 0; index < volumes.size(); ++index)
                    if (volumes[index].fileSystem) { selectedVolume = index; selectedSession = volumes[index].descriptor.sessionNumber; break; }
            }
            folderPath.clear();
        }
    } catch (const MediaEngine::MediaError& error) {
        if (!cancelled_) {
            error_ = QString::fromUtf8(error.what());
            errorMessage_ = MediaEngine::MessageInfo{std::string(MediaEngine::diagnosticKey(error.diagnostic().code)), error.diagnostic().arguments, MediaDomain};
        }
    } catch (const std::exception& error) {
        if (!cancelled_) error_ = QString::fromUtf8(error.what());
    } catch (...) {
        if (!cancelled_) error_ = QString::fromLatin1(UnexpectedError);
    }
    notify();
}
ExplorerSession::Subscription ExplorerSession::subscribe(std::function<void()> callback) {
    auto value = std::make_shared<const std::function<void()>>(std::move(callback));
    observers_.push_back(value);
    return value;
}
void ExplorerSession::notify() {
    const auto observers = observers_;
    std::erase_if(observers_, [](const auto& observer) { return observer.expired(); });
    for (const auto& observer : observers) if (const auto callback = observer.lock()) (*callback)();
}
}
