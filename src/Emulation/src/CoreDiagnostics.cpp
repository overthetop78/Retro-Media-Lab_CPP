#include <RetroMediaLab/Emulation/CoreDiagnostics.h>
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QtEndian>
#include <algorithm>
#include <ios>
#include <utility>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#include <winver.h>
#endif

namespace RetroMediaLab::Emulation {
namespace {
#ifdef Q_OS_WIN
constexpr auto DiagnosticNewline = QLatin1String("\r\n");
#else
constexpr auto DiagnosticNewline = QLatin1String("\n");
#endif

constexpr quint16 DosSignature = 0x5A4D, Pe32 = 0x10B, Pe32Plus = 0x20B;
constexpr quint32 PeSignature = 0x00004550;
constexpr quint16 MachineX64 = 0x8664, MachineX86 = 0x014C, MachineArm64 = 0xAA64;
constexpr quint64 DosHeaderSize = 64, PePointer = 60, BasicPeSize = 6, CoffHeaderSize = 24;
constexpr int SectionCount = 6, OptionalSize = 20;
constexpr int ExportEntryPe32 = 96, ExportEntryPe32Plus = 112, DirectoryEntrySize = 8, DirectoryCountSize = 4;
constexpr int ExportDirectorySize = 40, ExportNameCount = 24, ExportNamePointers = 32;
constexpr int SectionSize = 40, SectionRva = 12, RawSectionSize = 16, RawSectionOffset = 20;
constexpr int PointerSize = sizeof(quint32);
constexpr quint32 MaximumExportNames = 65536;
constexpr quint64 MaximumExportNameBytes = 16384;
constexpr auto RejectedCoreKey = "Emulation.Error.EmulatorRejected";
constexpr auto UnknownMetadata = QLatin1String("unknown");

QFile& openCore(QFile& file) {
    if (!file.open(QIODevice::ReadOnly)) throw std::ios_base::failure(RejectedCoreKey);
    return file;
}

QString hashCore(QFile& file) {
    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!file.seek(0) || !hash.addData(&file)) throw std::ios_base::failure(RejectedCoreKey);
    return QString::fromLatin1(hash.result().toHex().toUpper());
}

template<typename T>
T peValue(QByteArrayView bytes, int offset = 0) {
    return qFromLittleEndian<T>(bytes.data() + offset);
}

class PeFile {
public:
    enum class HeaderIssue { None, NotPe, InvalidOffset, WrongSignature };

    explicit PeFile(QFile& file) : file_(file), size_(file.size()) {
        const auto dos = read(0, DosHeaderSize);
        if (dos.isEmpty() || peValue<quint16>(dos) != DosSignature) {
            issue = HeaderIssue::NotPe;
            return;
        }
        offset_ = peValue<quint32>(dos, PePointer);
        const auto header = offset_ >= DosHeaderSize ? read(offset_, BasicPeSize) : QByteArray{};
        if (header.isEmpty()) issue = HeaderIssue::InvalidOffset;
        else if (peValue<quint32>(header) != PeSignature) issue = HeaderIssue::WrongSignature;
        else machine = peValue<quint16>(header, sizeof(quint32));
    }

    QStringList exports() {
        if (issue != HeaderIssue::None) return {};
        const auto coff = read(offset_, CoffHeaderSize);
        if (coff.isEmpty()) return {};
        const auto optionalLength = peValue<quint16>(coff, OptionalSize);
        const auto optional = read(offset_ + CoffHeaderSize, optionalLength);
        if (optional.size() < qsizetype(sizeof(quint16))) return {};
        const auto magic = peValue<quint16>(optional);
        if (magic != Pe32 && magic != Pe32Plus) return {};
        const int exportEntry = magic == Pe32 ? ExportEntryPe32 : ExportEntryPe32Plus;
        if (optional.size() < exportEntry + DirectoryEntrySize ||
            !peValue<quint32>(optional, exportEntry - DirectoryCountSize)) return {};
        const auto exportRva = peValue<quint32>(optional, exportEntry);
        if (!exportRva || peValue<quint32>(optional, exportEntry + PointerSize) < ExportDirectorySize) return {};
        sections_ = read(offset_ + CoffHeaderSize + optionalLength,
                         quint64(peValue<quint16>(coff, SectionCount)) * SectionSize);
        const auto directory = readRva(exportRva, ExportDirectorySize);
        if (directory.isEmpty()) return {};
        const auto count = peValue<quint32>(directory, ExportNameCount);
        if (!count || count > MaximumExportNames) return {};
        const auto pointers = readRva(peValue<quint32>(directory, ExportNamePointers), quint64(count) * PointerSize);
        if (pointers.isEmpty()) return {};
        QStringList result;
        result.reserve(count);
        for (quint32 index = 0; index < count; ++index) {
            const auto range = locate(peValue<quint32>(pointers, int(index * PointerSize)));
            if (!range) return {};
            const auto [start, available] = *range;
            const auto name = read(start, std::min(available, MaximumExportNameBytes));
            const auto ending = name.indexOf('\0');
            if (ending <= 0) return {};
            result.append(QString::fromLatin1(name.constData(), ending));
        }
        std::ranges::sort(result);
        return result;
    }

    HeaderIssue issue = HeaderIssue::None;
    quint16 machine = 0;

private:
    QByteArray read(quint64 offset, quint64 length) {
        if (offset > size_ || length > size_ - offset || !file_.seek(qint64(offset))) return {};
        auto bytes = file_.read(qint64(length));
        return quint64(bytes.size()) == length ? bytes : QByteArray{};
    }

    std::optional<std::pair<quint64, quint64>> locate(quint32 rva) const {
        if (!rva) return std::nullopt;
        for (int index = 0; index < sections_.size(); index += SectionSize) {
            const auto base = peValue<quint32>(sections_, index + SectionRva);
            const auto length = peValue<quint32>(sections_, index + RawSectionSize);
            if (rva < base || rva - base >= length) continue;
            const auto start = quint64(peValue<quint32>(sections_, index + RawSectionOffset)) + (rva - base);
            if (start >= size_) return std::nullopt;
            return std::pair(start, std::min(quint64(length - (rva - base)), size_ - start));
        }
        return std::nullopt;
    }

    QByteArray readRva(quint32 rva, quint64 length) {
        const auto range = locate(rva);
        return range && length <= range->second ? read(range->first, length) : QByteArray{};
    }

    QFile& file_;
    const quint64 size_;
    quint64 offset_ = 0;
    QByteArray sections_;
};

QString architecture(quint16 machine) {
    switch (machine) {
    case MachineX64: return QStringLiteral("x64");
    case MachineX86: return QStringLiteral("x86");
    case MachineArm64: return QStringLiteral("arm64");
    default: return UnknownMetadata;
    }
}

QString declaredVersion(const QString& path) {
#ifdef Q_OS_WIN
    constexpr auto TranslationBlock = L"\\VarFileInfo\\Translation";
    constexpr auto FileVersionBlock = "\\StringFileInfo\\%1%2\\FileVersion";
    constexpr auto VersionCodeWidth = 4;
    constexpr auto HexBase = 16;
    constexpr DWORD MaximumVersionBytes = 16 * 1024 * 1024;
    struct Translation { WORD language, codePage; };
    const auto filename = QFileInfo(path).absoluteFilePath().toStdWString();
    const auto size = GetFileVersionInfoSizeW(filename.c_str(), nullptr);
    if (size && size <= MaximumVersionBytes) {
        QByteArray data(size, Qt::Uninitialized);
        void* translations = nullptr;
        UINT length = 0;
        if (GetFileVersionInfoW(filename.c_str(), 0, size, data.data()) &&
            VerQueryValueW(data.constData(), TranslationBlock, &translations, &length)) {
            const auto* entries = static_cast<const Translation*>(translations);
            for (UINT index = 0; index < length / sizeof(Translation); ++index) {
                const auto query = QString::fromLatin1(FileVersionBlock)
                    .arg(entries[index].language, VersionCodeWidth, HexBase, u'0')
                    .arg(entries[index].codePage, VersionCodeWidth, HexBase, u'0').toStdWString();
                void* result = nullptr;
                UINT characters = 0;
                if (VerQueryValueW(data.constData(), query.c_str(), &result, &characters) && characters > 1)
                    return QString::fromWCharArray(static_cast<const wchar_t*>(result), characters - 1);
            }
        }
    }
#else
    Q_UNUSED(path);
#endif
    return UnknownMetadata;
}
}

namespace CoreDiagnostics {
void requireArchitecture(const QString& actual, const QString& expected, const ErrorText& error) {
    if (actual != expected) throw std::invalid_argument(error().toStdString());
}

void verifyWindowsX64Library(const QString& path, const ErrorText& notPe, const ErrorText& invalidPe,
                            const ErrorText& wrongArchitecture) {
    QFile file(path);
    const PeFile pe(openCore(file));
    if (pe.issue == PeFile::HeaderIssue::NotPe) throw std::invalid_argument(notPe().toStdString());
    if (pe.issue == PeFile::HeaderIssue::InvalidOffset) throw std::invalid_argument(invalidPe().toStdString());
    if (pe.issue != PeFile::HeaderIssue::None || pe.machine != MachineX64)
        throw std::invalid_argument(wrongArchitecture().toStdString());
}

QString calculateSha256(const QString& path) {
    QFile file(path);
    return hashCore(openCore(file));
}

Manifest createManifest(const QString& releaseId, const QString& releaseVersion, const QString& downloadUrl,
                        qint64 archiveSize, const QString& libraryPath) {
    QFile file(libraryPath);
    PeFile pe(openCore(file));
    return {releaseId, releaseVersion, downloadUrl, QDateTime::currentDateTimeUtc(), archiveSize,
            file.size(), hashCore(file), architecture(pe.machine), declaredVersion(libraryPath), pe.exports()};
}

QString findInstalledLibraryPath(const QString& path, const std::function<void(const QString&)>& verifyInstallation,
                                 const std::function<bool()>& isInstallationComplete) {
    if (!QFileInfo(path).isFile() || (isInstallationComplete && !isInstallationComplete())) return {};
    try {
        if (verifyInstallation) verifyInstallation(path);
        return path;
    } catch (const std::ios_base::failure&) {
        return {};
    } catch (const std::invalid_argument&) {
        return {};
    }
}
}

CoreDiagnosticMonitor::CoreDiagnosticMonitor(QStringList signatures, FailureFactory createFailure)
    : signatures_(std::move(signatures)), createFailure_(std::move(createFailure)) {}

void CoreDiagnosticMonitor::capture(QStringView message) noexcept {
    try {
        std::lock_guard lock(mutex_);
        if (failure_) return;
        for (const auto& signature : signatures_) {
            if (message.contains(signature, Qt::CaseSensitive)) {
                failure_.emplace(message.toString());
                return;
            }
        }
    } catch (...) {
        // A native callback cannot propagate a C++ exception to the core.
    }
}

std::optional<QString> CoreDiagnosticMonitor::failure() const {
    std::lock_guard lock(mutex_);
    return failure_;
}

void CoreDiagnosticMonitor::throwIfFailed() const {
    if (const auto diagnostic = failure(); diagnostic && createFailure_) {
        if (auto error = createFailure_(*diagnostic)) std::rethrow_exception(error);
    }
}

NativeDiagnosticsError::NativeDiagnosticsError(const QString& message, std::exception_ptr cause)
    : std::runtime_error(message.toStdString()), cause_(std::move(cause)) {}

const std::exception_ptr& NativeDiagnosticsError::cause() const noexcept { return cause_; }

std::exception_ptr withNativeDiagnostics(std::exception_ptr cause, const QStringList& diagnostics) {
    if (!cause || diagnostics.isEmpty()) return cause;
    try {
        std::rethrow_exception(cause);
    } catch (const std::exception& error) {
        const auto block = diagnostics.join(DiagnosticNewline);
        const auto message = QString::fromUtf8(error.what());
        if (message.contains(block, Qt::CaseSensitive)) return cause;
        return std::make_exception_ptr(NativeDiagnosticsError(message + DiagnosticNewline + block, cause));
    } catch (...) {
        return cause;
    }
}
}
