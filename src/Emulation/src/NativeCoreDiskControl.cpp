#include <RetroMediaLab/Emulation/NativeCoreDiskControl.h>
#include <QFileInfo>
#include <algorithm>
#include <array>
#include <limits>
#include <stdexcept>
#include <utility>

namespace RetroMediaLab::Emulation {
namespace {
constexpr auto ErrorKey = "Emulation.Error.EmulatorRejected";
constexpr auto NoNativeIndex = std::numeric_limits<unsigned>::max();
constexpr int NoIndex = -1;
constexpr std::size_t TextCapacity = 4096;

void require(bool accepted) {
    if (!accepted) throw std::runtime_error(ErrorKey);
}

int checkedIndex(unsigned index) {
    if (!std::in_range<int>(index)) throw std::overflow_error(ErrorKey);
    return static_cast<int>(index);
}

QString readText(NativeCoreDiskExtendedCallbacks::ImageText getter, int index) {
    if (!getter || index < 0) return {};
    std::array<char, TextCapacity> buffer{};
    if (!getter(static_cast<unsigned>(index), buffer.data(), buffer.size())) return {};
    const auto end = std::ranges::find(buffer, '\0');
    return QString::fromUtf8(buffer.data(), end - buffer.begin());
}
}

NativeCoreDiskControl::NativeCoreDiskControl(bool resetOnEmptyCapture, MediaExists mediaExists)
    : resetOnEmptyCapture_(resetOnEmptyCapture), mediaExists_(std::move(mediaExists)) {}

void NativeCoreDiskControl::capture(const NativeCoreDiskCallbacks& api) {
    auto extended = callbacks_;
    extended.basic = api;
    capture(extended);
}

void NativeCoreDiskControl::capture(const NativeCoreDiskExtendedCallbacks& api) {
    const auto& basic = api.basic;
    if (resetOnEmptyCapture_ && !basic.setEjectState && !basic.getEjectState && !basic.getImageIndex &&
        !basic.setImageIndex && !basic.getImageCount && !basic.replaceImage && !basic.addImage) {
        clear();
        return;
    }
    require(basic.setEjectState && basic.getEjectState && basic.getImageIndex && basic.setImageIndex &&
        basic.getImageCount && basic.replaceImage && basic.addImage);
    callbacks_ = api;
}

void NativeCoreDiskControl::clear() noexcept { callbacks_ = {}; }
bool NativeCoreDiskControl::available() const noexcept { return callbacks_.basic.setEjectState != nullptr; }
int NativeCoreDiskControl::imageCount() const { return available() ? checkedIndex(callbacks_.basic.getImageCount()) : 0; }
bool NativeCoreDiskControl::ejected() const { return !available() || callbacks_.basic.getEjectState(); }

int NativeCoreDiskControl::currentIndex() const {
    if (!available()) return NoIndex;
    const auto index = callbacks_.basic.getImageIndex();
    return index == NoNativeIndex ? NoIndex : checkedIndex(index);
}

QString NativeCoreDiskControl::path(int index) const { return readText(callbacks_.getImagePath, index); }
QString NativeCoreDiskControl::label(int index) const { return readText(callbacks_.getImageLabel, index); }

DiskStatus NativeCoreDiskControl::status() const {
    DiskStatus result;
    const auto count = imageCount();
    for (int index = 0; index < count; ++index) {
        auto image = label(index);
        if (image.isEmpty()) image = path(index);
        result.images.append(std::move(image));
    }
    result.currentIndex = currentIndex();
    result.ejected = ejected();
    return result;
}

void NativeCoreDiskControl::ensureAvailable() const { require(available()); }

void NativeCoreDiskControl::ensureMediaExists(const QString& path, bool richContent) const {
    if (mediaExists_) {
        require(mediaExists_(path));
        return;
    }
    const QFileInfo file(path);
    require(file.isFile() || (!richContent && file.isDir()));
}

void NativeCoreDiskControl::recover(bool wasEjected) const noexcept {
    if (!wasEjected) {
        try { callbacks_.basic.setEjectState(false); } catch (...) {}
    }
}

void NativeCoreDiskControl::select(int index) const {
    ensureAvailable();
    const auto count = imageCount();
    if (index < 0 || index >= count) throw std::out_of_range(ErrorKey);
    const auto& api = callbacks_.basic;
    const auto previous = api.getImageIndex();
    const bool wasEjected = api.getEjectState();
    if (!wasEjected) require(api.setEjectState(true));
    try {
        require(api.setImageIndex(static_cast<unsigned>(index)));
        require(api.setEjectState(false));
    } catch (...) {
        if (previous != NoNativeIndex) {
            try { api.setImageIndex(previous); } catch (...) {}
        }
        recover(wasEjected);
        throw;
    }
}

void NativeCoreDiskControl::insert(const QString& path, const ContentFactory& createContent) const {
    ensureMediaExists(path, bool(createContent));
    ensureAvailable();
    const auto& api = callbacks_.basic;
    const bool wasEjected = api.getEjectState();
    if (!wasEjected) require(api.setEjectState(true));
    try {
        const auto count = api.getImageCount();
        const auto index = count == 0 ? 0u : std::min(api.getImageIndex(), count - 1);
        if (count == 0) require(api.addImage());
        auto content = createContent ? createContent(path) :
            std::make_unique<NativeCoreContent>(QFileInfo(path).absoluteFilePath(), true);
        require(bool(content));
        require(api.replaceImage(index, &content->gameInfo()));
        require(api.setImageIndex(index));
        require(api.setEjectState(false));
    } catch (...) {
        recover(wasEjected);
        throw;
    }
}

void NativeCoreDiskControl::insertAt(int index, const QString& path, int firstImageIndex,
    const ContentFactory& createContent) const {
    ensureAvailable();
    if (index < firstImageIndex || index < 0) throw std::out_of_range(ErrorKey);
    ensureMediaExists(path, false);
    const auto& api = callbacks_.basic;
    const bool wasEjected = api.getEjectState();
    if (!wasEjected) require(api.setEjectState(true));
    try {
        auto count = imageCount();
        while (count <= index) {
            require(api.addImage());
            const auto nextCount = imageCount();
            require(nextCount > count);
            count = nextCount;
        }
        const auto slot = static_cast<unsigned>(index);
        require(api.setImageIndex(slot));
        auto content = createContent(path);
        require(bool(content));
        require(api.replaceImage(slot, &content->gameInfo()));
        require(api.setEjectState(false));
    } catch (...) {
        recover(wasEjected);
        throw;
    }
}

void NativeCoreDiskControl::eject() const {
    ensureAvailable();
    require(callbacks_.basic.setEjectState(true));
}
}
