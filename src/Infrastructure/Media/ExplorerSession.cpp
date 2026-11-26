#include "ExplorerSession.h"
#include "MediaSessionConstants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <chrono>
#include <algorithm>
#include <exception>
#include <QFileInfo>

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
    pending_={};pendingLifetime_.reset();
}
void ExplorerSession::open(const QString& path, std::optional<std::string> format, std::shared_ptr<const void> lifetime) {
    startOpening(path, std::move(format), std::move(lifetime), false);
}
void ExplorerSession::startOpening(const QString& path, std::optional<std::string> format,
        std::shared_ptr<const void> lifetime, bool trial) {
    if (busy() || path.isEmpty()) return;
    trial_ = trial;
    cancellation_ = std::stop_source{};
    cancelled_ = false;
    error_.clear();
    errorMessage_.reset();
    pendingLifetime_=std::move(lifetime);
    pendingPath_ = path;
    pendingFormat_ = format;
    progress_=std::make_shared<OpeningProgress>();progress_->reset(QFileInfo(path).fileName().toStdString());progressRevision_=0;
    try {
        const auto progress=progress_;
        pending_ = MediaEngine::analyzeMediaOpeningAsync(std::filesystem::path(path.toStdU16String()), std::move(format), [progress](const auto& item){progress->report(item);}, [progress](const auto& item){progress->report(item);}, cancellation_.get_token(),true,[progress](auto capture){progress->loadCapture(std::move(capture));});
        timer_.start();
    } catch (const MediaEngine::MediaError& error) {
        if (!trial_) {
            error_ = QString::fromUtf8(error.what());
            errorMessage_ = MediaEngine::MessageInfo{std::string(MediaEngine::diagnosticKey(error.diagnostic().code)), error.diagnostic().arguments, MediaDomain};
        }
    } catch (const std::exception& error) {
        if (!trial_) error_ = QString::fromUtf8(error.what());
    } catch (...) {
        if (!trial_) error_ = QString::fromLatin1(UnexpectedError);
    }
    if(!pending_.valid())pendingLifetime_.reset();
    notify();
}
void ExplorerSession::reclassify(bool automaticChanged) {
    if (busy()) return;
    if (path_.isEmpty()) { notify(); return; }
    const auto format = automaticChanged && classification.automatic() ? std::optional<std::string>{} : classification.format();
    if (result_ && result_->diskExploration) {
        const auto& disk = result_->diskExploration->info();
        if (!format || std::any_of(disk.formats.begin(), disk.formats.end(),
                [&](const auto& item) { return item.formatId == *format; })) {
            if (selectedFormat != format) {
                selectedVolume = 0;
                selectedSession.reset();
                folderPath.clear();
                selectedEntry.reset();
            }
            selectedFormat = format;
            error_.clear();
            errorMessage_.reset();
            notify();
            return;
        }
    }
    startOpening(path_,format,result_,true);
}
void ExplorerSession::cancel() {
    if (!busy()) return;
    cancelled_ = true;
    cancellation_.request_stop();
}
void ExplorerSession::poll() {
    if(!pending_.valid())return;
    if(pending_.wait_for(std::chrono::seconds::zero())!=std::future_status::ready){const auto revision=progress_->snapshot().revision;if(revision!=progressRevision_){progressRevision_=revision;notify();}return;}
    timer_.stop();
    try {
        auto value = pending_.get();
        const auto hasPresentation = [&] {
            if (!trial_) return true;
            if (!value.diskExploration || !pendingFormat_) return false;
            const auto& disk = value.diskExploration->info();
            return std::any_of(disk.formats.begin(), disk.formats.end(), [&](const auto& item) {
                return item.formatId == *pendingFormat_;
            });
        };
        if (!cancelled_ && hasPresentation()) {
            if(pendingLifetime_){
                struct OwnedDocument {std::shared_ptr<const void> lifetime;std::shared_ptr<const MediaEngine::MediaDocument> document;};
                const auto retain=[&](auto& document){if(!document)return;auto owner=std::make_shared<OwnedDocument>(OwnedDocument{pendingLifetime_,document});document=std::shared_ptr<const MediaEngine::MediaDocument>(owner,owner->document.get());};
                retain(value.document);if(value.mediaExploration)retain(value.mediaExploration->document);
            }
            struct OwnedResult {std::shared_ptr<const void> lifetime;MediaEngine::MediaOpeningAnalysisResult result;};
            auto owner=std::make_shared<OwnedResult>(OwnedResult{std::move(pendingLifetime_),std::move(value)});result_=std::shared_ptr<const MediaEngine::MediaOpeningAnalysisResult>(owner,&owner->result);
            path_ = pendingPath_;
            selectedFormat = pendingFormat_;
            if(result_->diskExploration)classification.detect(result_->diskExploration->info());
            selectedVolume = 0;
            selectedSession.reset();
            if (result_->mediaExploration) {
                const auto& volumes = result_->mediaExploration->volumes;
                for (std::size_t index = 0; index < volumes.size(); ++index)
                    if (volumes[index].fileSystem) { selectedVolume = index; selectedSession = volumes[index].descriptor.sessionNumber; break; }
            }
            folderPath.clear(); selectedEntry.reset();
        }
    } catch (const MediaEngine::MediaError& error) {
        if (!cancelled_ && !trial_) {
            error_ = QString::fromUtf8(error.what());
            errorMessage_ = MediaEngine::MessageInfo{std::string(MediaEngine::diagnosticKey(error.diagnostic().code)), error.diagnostic().arguments, MediaDomain};
        }
    } catch (const std::exception& error) {
        if (!cancelled_ && !trial_) error_ = QString::fromUtf8(error.what());
    } catch (...) {
        if (!cancelled_ && !trial_) error_ = QString::fromLatin1(UnexpectedError);
    }
    if (!cancelled_ && trial_) {
        selectedFormat = pendingFormat_;
        classification.selectFormat(selectedFormat);
        selectedVolume = 0;
        selectedSession.reset();
        folderPath.clear();
        selectedEntry.reset();
    }
    pendingLifetime_.reset();
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
