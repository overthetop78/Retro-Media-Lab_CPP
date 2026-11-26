#include <RetroMediaLab/Emulation/NativeCoreContent.h>
#include <QFile>
#include <algorithm>
#include <stdexcept>

namespace RetroMediaLab::Emulation {
namespace {
constexpr auto ErrorKey = "Emulation.Error.EmulatorRejected";
constexpr char Terminator = '\0';
constexpr qsizetype TerminatorSize = 1;

QByteArray readCoreContent(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) throw std::runtime_error(ErrorKey);
    auto data = file.readAll();
    if (file.error() != QFileDevice::NoError) throw std::runtime_error(ErrorKey);
    return data;
}
}

NativeCoreString::NativeCoreString(QStringView value, qsizetype minimumByteCapacity,
    NativeStringEncoding encoding) {
    if (value.isNull() || minimumByteCapacity < 0) throw std::invalid_argument(ErrorKey);
#ifdef Q_OS_WIN
    bytes_ = encoding == NativeStringEncoding::WindowsAnsi ? value.toLocal8Bit() : value.toUtf8();
#else
    Q_UNUSED(encoding);
    bytes_ = value.toUtf8();
#endif
    bytes_.resize(std::max(bytes_.size() + TerminatorSize, minimumByteCapacity), Terminator);
}

const char* NativeCoreString::pointer() const noexcept { return bytes_.constData(); }

const NativeCoreString& addNativeCoreString(NativeCoreStrings& owner, const QString& key, QStringView value) {
    const auto [entry, added] = owner.try_emplace(key, value);
    if (!added) throw std::invalid_argument(ErrorKey);
    return entry->second;
}

NativeCoreContent::NativeCoreContent(const QString& path, bool needsFullPath, NativeStringEncoding pathEncoding)
    : path_(path, 0, pathEncoding),
      data_(needsFullPath ? QByteArray{} : readCoreContent(path)),
      info_{path_.pointer(), needsFullPath ? nullptr : data_.constData(), std::size_t(data_.size()), nullptr} {}

const NativeCoreGameInfo& NativeCoreContent::gameInfo() const noexcept { return info_; }
}
