#include "MediaPreparation.h"
#include <RetroMediaLab/Emulation/StorageSettings.h>
#include <RetroMediaLab/MediaEngine/MediaExploration.h>
#include <RetroMediaLab/MediaEngine/MediaPreparation.h>
#include <RetroMediaLab/MediaEngine/OpticalDiscSession.h>
#include <RetroMediaLab/MediaEngine/RuntimeDisk.h>
#include <RetroMediaLab/MediaEngine/TapePreparation.h>
#include <QDir>
#include <QFileInfo>
#include <QSaveFile>
#include <QTemporaryDir>
#include <stdexcept>

namespace RetroMediaLab::Emulation {
namespace {
std::filesystem::path mediaPath(const QString& path) { return std::filesystem::path(path.toStdU16String()); }
QByteArray byteArray(const std::vector<std::uint8_t>& bytes) {
    return QByteArray(reinterpret_cast<const char*>(bytes.data()), qsizetype(bytes.size()));
}

class DiskImage final : public IEmulationDiskImage {
public:
    explicit DiskImage(std::unique_ptr<MediaEngine::RuntimeDisk> disk) : disk_(std::move(disk)) {}
    QString format() const override { return QString::fromStdString(disk_->formatId()); }
    DiskGeometry geometry() const override {
        return {disk_->cylinders(), disk_->sectorCount(), disk_->heads(), disk_->sectorsPerTrack(), disk_->sectorSize()};
    }
    bool readOnly() const override { return disk_->isReadOnly(); }
    bool hasChanges() const override { return disk_->hasChanges(); }
    std::optional<qint64> findSector(int cylinder, int head, int number) const override {
        return disk_->findSector(cylinder, head, number);
    }
    DiskSector read(qint64 index, std::stop_token token) const override {
        auto sector = disk_->readSectorAsync(index, token).get();
        return {sector.index, byteArray(sector.data), sector.cylinder, sector.head, sector.number,
            sector.integrityValid, sector.deletedDataMark};
    }
    void write(qint64 index, const QByteArray& bytes, std::optional<bool> deleted) override {
        disk_->writeSector(index, {reinterpret_cast<const std::uint8_t*>(bytes.constData()), std::size_t(bytes.size())}, deleted);
    }
    void flush(std::stop_token token) override { disk_->flushAsync(token).get(); }
    void close() override { disk_->close(); }
private:
    std::unique_ptr<MediaEngine::RuntimeDisk> disk_;
};

class OpticalDisc final : public IEmulationOpticalDisc {
public:
    explicit OpticalDisc(std::unique_ptr<MediaEngine::OpticalDiscSession> disc)
        : disc_(std::move(disc)), descriptors_(disc_->tracks()) {}
    QString format() const override { return QString::fromStdString(disc_->formatId()); }
    QList<OpticalTrack> tracks() const override {
        QList<OpticalTrack> result;
        for (const auto& track : descriptors_)
            result.push_back({track.trackNumber, track.firstSector, track.sectorCount, track.storedSectorSize,
                track.userDataOffset, track.userDataLength, track.isAudio(), track.hasSubchannels});
        return result;
    }
    QByteArray readStoredSector(int index, qint64 sector, std::stop_token token) const override {
        const auto& track = descriptors_.at(std::size_t(index));
        QByteArray bytes(track.storedSectorSize, Qt::Uninitialized);
        disc_->readStoredSectorAsync(index, sector, {reinterpret_cast<std::uint8_t*>(bytes.data()), std::size_t(bytes.size())}, token).get();
        return bytes;
    }
    QByteArray readSubchannel(int index, qint64 sector, std::stop_token token) const override {
        const auto bytes = disc_->readSubchannelAsync(index, sector, token).get();
        return bytes ? byteArray(*bytes) : QByteArray{};
    }
private:
    std::unique_ptr<MediaEngine::OpticalDiscSession> disc_;
    std::vector<MediaEngine::OpticalDiscTrack> descriptors_;
};
}

QString MediaPreparation::detectFormat(const QString& path, std::stop_token token) const {
    auto result = MediaEngine::analyzeMediaOpeningAsync(mediaPath(path), {}, {}, {}, token).get();
    if (result.diskExploration && !result.diskExploration->info().detectedFormatIds.empty())
        return QString::fromStdString(result.diskExploration->info().primaryFormatId);
    return result.mediaExploration ? QString::fromStdString(result.mediaExploration->formatId) : QString{};
}
std::unique_ptr<IEmulationDiskImage> MediaPreparation::openDisk(const QString& path, bool writable,
        const QString& format, std::stop_token token) const {
    const auto id = format.isEmpty() ? std::nullopt : std::optional(format.toStdString());
    return std::make_unique<DiskImage>(MediaEngine::RuntimeDisk::openAsync(mediaPath(path), writable, id, token).get());
}
std::unique_ptr<IEmulationOpticalDisc> MediaPreparation::openOptical(const QString& path, std::stop_token token) const {
    return std::make_unique<OpticalDisc>(MediaEngine::OpticalDiscSession::openAsync(mediaPath(path), token, true).get());
}
void MediaPreparation::createBlankDisk(const QString& path, const QString& format, std::stop_token token) const {
    MediaEngine::createBlankDiskAsync(mediaPath(path), format.toStdString(), token).get();
}

void MediaPreparation::createHardDisk(const QString& path, const QString& formatId,
        const HardDiskImageGeometry& geometry, const QString& fileSystemId, bool preallocate, std::stop_token token) const {
    std::optional<std::string> fileSystem;
    if (!fileSystemId.isEmpty()) fileSystem = fileSystemId.toStdString();
    MediaEngine::createHardDiskAsync(mediaPath(path), formatId.toStdString(),
        MediaEngine::HardDiskGeometry(geometry.cylinders, geometry.heads, geometry.sectorsPerTrack, geometry.sectorSize),
        std::move(fileSystem), token, preallocate).get();
}

QStringList MediaPreparation::tapeRecordingExtensions(const MediaSlot& slot) const {
    QStringList extensions;
    if (slot.kind != MediaKind::Tape) return extensions;
    for (const auto& suffix : MediaEngine::tapeRecordingExtensions()) {
        const auto extension = QString::fromStdString(suffix);
        if (slot.acceptedExtensions.contains(extension, Qt::CaseInsensitive)) extensions.append(extension);
    }
    return extensions;
}

void MediaPreparation::createBlankTape(const QString& path, std::optional<TapePcmSettings> pcm,
        std::stop_token token) const {
    const QFileInfo target(path);
    constexpr auto TemporaryTapeDirectory = ".rml-tape-XXXXXX";
    constexpr auto FileError = "Emulation.Error.EmulatorRejected";
    constexpr qint64 MaximumBlankTapeSize = 1024 * 1024;
    QTemporaryDir staging(target.dir().filePath(QLatin1String(TemporaryTapeDirectory)));
    if (!staging.isValid()) throw std::runtime_error(FileError);
    const auto stagingPath = staging.filePath(target.fileName());
    std::map<std::string, std::string> parameters;
    if (pcm) {
        const auto values = pcm->parameters();
        for (auto it = values.cbegin(); it != values.cend(); ++it)
            parameters.emplace(it.key().toStdString(), it.value().toStdString());
    }
    const auto rate = pcm ? pcm->sampleRate : TapePcmSettings::DefaultSampleRate;
    MediaEngine::writeTapeAsync(mediaPath(stagingPath), MediaEngine::MediaTapeSignal({}, rate), token,
        std::move(parameters)).get();
    QFile source(stagingPath);
    QSaveFile destination(path);
    if (token.stop_requested() || !source.open(QIODevice::ReadOnly) || source.size() > MaximumBlankTapeSize ||
        !destination.open(QIODevice::WriteOnly)) throw std::runtime_error(FileError);
    const auto bytes = source.readAll();
    if (source.error() != QFileDevice::NoError || destination.write(bytes) != bytes.size() ||
        token.stop_requested() || !destination.commit()) throw std::runtime_error(FileError);
}
}
