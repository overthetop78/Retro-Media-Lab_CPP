#pragma once

#include "EmulationExport.h"
#include <QByteArray>
#include <QString>
#include <QStringView>
#include <cstddef>
#include <map>

namespace RetroMediaLab::Emulation {
enum class NativeStringEncoding { Utf8, WindowsAnsi };

class RETROMEDIALAB_EMULATION_API NativeCoreString final {
public:
    explicit NativeCoreString(QStringView value, qsizetype minimumByteCapacity = 0,
        NativeStringEncoding encoding = NativeStringEncoding::Utf8);
    NativeCoreString(const NativeCoreString&) = delete;
    NativeCoreString& operator=(const NativeCoreString&) = delete;

    // The core borrows this address for the lifetime of the owner.
    const char* pointer() const noexcept;

private:
    QByteArray bytes_;
};

using NativeCoreStrings = std::map<QString, NativeCoreString>;
RETROMEDIALAB_EMULATION_API const NativeCoreString& addNativeCoreString(
    NativeCoreStrings& owner, const QString& key, QStringView value);

// Native layout of the core's game-info structure.
struct NativeCoreGameInfo {
    const char* path;
    const void* data;
    std::size_t size;
    const char* metadata;
};

class RETROMEDIALAB_EMULATION_API NativeCoreContent final {
public:
    NativeCoreContent(const QString& path, bool needsFullPath,
        NativeStringEncoding pathEncoding = NativeStringEncoding::Utf8);
    NativeCoreContent(const NativeCoreContent&) = delete;
    NativeCoreContent& operator=(const NativeCoreContent&) = delete;

    const NativeCoreGameInfo& gameInfo() const noexcept;

private:
    NativeCoreString path_;
    QByteArray data_;
    NativeCoreGameInfo info_;
};
}
