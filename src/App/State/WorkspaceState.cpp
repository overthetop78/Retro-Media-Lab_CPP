#include "WorkspaceState.h"
#include <algorithm>
#include <stdexcept>
namespace RetroMediaLab {
bool WorkspaceState::hasGeometry() const noexcept { return !mainWindowGeometry.isEmpty(); }
void WorkspaceState::setDefaultImageDirectory(QString directory) {
    if (directory.contains(QChar::Null)) throw std::invalid_argument("Invalid image directory");
    if (directory == defaultImageDirectory_) return;
    defaultImageDirectory_ = std::move(directory);
    const auto observers = directoryObservers_;
    std::erase_if(directoryObservers_, [](const auto& observer) { return observer.expired(); });
    for (const auto& observer : observers)
        if (const auto live = observer.lock()) (*live)();
}
WorkspaceState::DirectorySubscription WorkspaceState::subscribeToDefaultImageDirectory(std::function<void()> observer) {
    auto subscription = std::make_shared<const std::function<void()>>(std::move(observer));
    directoryObservers_.push_back(subscription);
    return subscription;
}
}
