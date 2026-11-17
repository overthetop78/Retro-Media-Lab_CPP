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
}
void VisualizationSession::sourceChanged() {
    if (!opening_->result() || opening_->result() == requested_) return;
    cancellation_.request_stop();
    if (pending_.valid() || inspection_.valid()) { timer_.start(); return; }
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
        if (value.descriptor->fields().representationKind == MediaEngine::RepresentationKind::Sequential) {
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
                inspectedElements.clear(); surface = 0; position = 0; revolution = 0; zoom = 1; panX = 0; panY = 0; selectedSector.reset();
            }
        } catch (const MediaEngine::MediaError& error) { if (!cancellation_.stop_requested()) captureError(error); }
          catch (const std::exception& error) { if (!cancellation_.stop_requested()) captureError(error); }
          catch (...) { if (!cancellation_.stop_requested()) error_ = MediaEngine::MessageInfo{UnexpectedError, {}, {}}; }
        changed = true;
    }
    if (inspection_.valid() && inspection_.wait_for(std::chrono::seconds::zero()) == std::future_status::ready) {
        try { const auto result = inspection_.get(); if (!cancellation_.stop_requested()) inspectedElements[inspecting_] = result; }
        catch (const MediaEngine::MediaError& error) { if (!cancellation_.stop_requested()) captureError(error); }
        catch (const std::exception& error) { if (!cancellation_.stop_requested()) captureError(error); }
        catch (...) { if (!cancellation_.stop_requested()) error_ = MediaEngine::MessageInfo{UnexpectedError, {}, {}}; }
        changed = true;
    }
    if (!pending_.valid() && !inspection_.valid()) { timer_.stop(); sourceChanged(); }
    if (changed) publish();
}
void VisualizationSession::select(std::int32_t side, std::int64_t track) { if (surface != side || position != track) selectedSector.reset(); surface = side; position = track; revolution = 0; publish(); }
void VisualizationSession::setZoom(double value) { if (std::isfinite(value) && value >= 1 && value <= MaximumZoom) { zoom = value; if (value == 1) { panX = 0; panY = 0; } publish(); } }
void VisualizationSession::setRevolution(std::size_t value) { revolution = value; publish(); }
void VisualizationSession::setPan(double x, double y) { if (std::isfinite(x) && std::isfinite(y)) { panX = x; panY = y; publish(); } }
void VisualizationSession::inspect(std::size_t element) {
    if (!snapshot_ || inspection_.valid() || pending_.valid() || element >= snapshot_->descriptor->fields().elements.size()) return;
    inspecting_ = element;
    inspection_ = MediaEngine::readBlockVisualizationElementAsync(snapshot_->document, snapshot_->descriptor->fields().elements[element], cancellation_.get_token());
    timer_.start();
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
