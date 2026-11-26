#pragma once

#include "NativeCoreContent.h"
#include "Sdk.h"
#include <functional>
#include <memory>

namespace RetroMediaLab::Emulation {
inline constexpr unsigned NativeCoreDiskControlVersion = 1;

struct NativeCoreDiskCallbacks {
    bool (*setEjectState)(bool);
    bool (*getEjectState)();
    unsigned (*getImageIndex)();
    bool (*setImageIndex)(unsigned);
    unsigned (*getImageCount)();
    bool (*replaceImage)(unsigned, const NativeCoreGameInfo*);
    bool (*addImage)();
};

struct NativeCoreDiskExtendedCallbacks {
    using ImageText = bool (*)(unsigned, char*, std::size_t);
    NativeCoreDiskCallbacks basic;
    bool (*setInitialImage)(unsigned, const char*);
    ImageText getImagePath;
    ImageText getImageLabel;
};

class RETROMEDIALAB_EMULATION_API NativeCoreDiskControl final {
public:
    using MediaExists = std::function<bool(const QString&)>;
    using ContentFactory = std::function<std::unique_ptr<NativeCoreContent>(const QString&)>;

    explicit NativeCoreDiskControl(bool resetOnEmptyCapture = false, MediaExists mediaExists = {});
    void capture(const NativeCoreDiskCallbacks&);
    void capture(const NativeCoreDiskExtendedCallbacks&);
    void clear() noexcept;

    bool available() const noexcept;
    int imageCount() const;
    int currentIndex() const;
    bool ejected() const;
    QString path(int index) const;
    QString label(int index) const;
    DiskStatus status() const;

    void select(int index) const;
    void insert(const QString& path, const ContentFactory& createContent = {}) const;
    void insertAt(int index, const QString& path, int firstImageIndex, const ContentFactory&) const;
    void eject() const;

private:
    void ensureAvailable() const;
    void ensureMediaExists(const QString&, bool richContent) const;
    void recover(bool wasEjected) const noexcept;

    bool resetOnEmptyCapture_;
    MediaExists mediaExists_;
    NativeCoreDiskExtendedCallbacks callbacks_{};
};
}
