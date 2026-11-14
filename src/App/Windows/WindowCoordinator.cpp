#include "WindowCoordinator.h"
#include <stdexcept>
namespace RetroMediaLab {
WindowCoordinator::WindowCoordinator(Factory factory) : factory_(std::move(factory)) {}
WindowCoordinator::~WindowCoordinator() = default;
void WindowCoordinator::rebuild() { replace(factory_); }
void WindowCoordinator::replace(Factory factory) {
    if (presentation_) presentation_->synchronizeState();
    auto candidate = factory();
    if (!candidate) throw std::runtime_error("Theme returned no presentation");
    candidate->show();
    // Nothing owning the current views is released until preparation succeeds.
    presentation_.swap(candidate);
    factory_.swap(factory);
    // Old windows and their subscriptions are destroyed here without a user close event.
}
}
