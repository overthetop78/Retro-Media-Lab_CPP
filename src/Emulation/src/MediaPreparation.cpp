#include "MediaPreparation.h"
#include <RetroMediaLab/Emulation/StorageSettings.h>
#include <RetroMediaLab/Emulation/Messages.h>
#include <RetroMediaLab/MediaEngine/CuePreparation.h>
#include <RetroMediaLab/MediaEngine/MechanicalSound.h>
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
QByteArray byteArray(std::span<const std::uint8_t> bytes) {
    return QByteArray(reinterpret_cast<const char*>(bytes.data()), qsizetype(bytes.size()));
}

std::span<const std::uint8_t> byteSpan(const QByteArray& bytes) {
    return {reinterpret_cast<const std::uint8_t*>(bytes.constData()), std::size_t(bytes.size())};
}
DiskSector diskSector(const MediaEngine::MediaDiskSector& sector) {
    return {sector.index, byteArray(sector.data), sector.cylinder, sector.head, sector.number,
        sector.integrityValid, sector.deletedDataMark};
}
template<class T>
std::map<std::string, T> nativeMap(const QMap<QString, T>& values) {
    std::map<std::string, T> result;
    for (auto it = values.cbegin(); it != values.cend(); ++it)
        result.emplace(it.key().toStdString(), it.value());
    return result;
}
std::optional<std::map<std::string, int>> fluxAttributes(const QMap<QString, int>& values) {
    return values.isEmpty() ? std::nullopt : std::optional(nativeMap(values));
}

std::optional<std::string> optionalString(const QString& value) {
    return value.isEmpty() ? std::nullopt : std::optional(value.toStdString());
}
QMap<QString, QString> qtMap(const std::map<std::string, std::string>& values) {
    QMap<QString, QString> result;
    for (const auto& [key, value] : values)
        result.insert(QString::fromStdString(key), QString::fromStdString(value));
    return result;
}
std::map<std::string, std::string> stringMap(const QMap<QString, QString>& values) {
    std::map<std::string, std::string> result;
    for (auto it = values.cbegin(); it != values.cend(); ++it)
        result.emplace(it.key().toStdString(), it.value().toStdString());
    return result;
}
QStringList qtStrings(const std::vector<std::string>& values) {
    QStringList result;
    result.reserve(qsizetype(values.size()));
    for (const auto& value : values) result.append(QString::fromStdString(value));
    return result;
}
MediaEngine::MediaKind mechanicalMediaKind(MediaKind kind) {
    using Target = MediaEngine::MediaKind;
    switch (kind) {
    case MediaKind::Floppy: return Target::Floppy;
    case MediaKind::HardDisk: return Target::HardDisk;
    case MediaKind::OpticalDisc: return Target::Optical;
    case MediaKind::Tape: return Target::Tape;
    default: throw EmulationMessageException({MessageCategory::Media, MessageCode::MediaUnsupported});
    }
}
class CueSheet final : public IEmulationCueSheet {
public:
    explicit CueSheet(MediaEngine::CueDocument document) : document_(std::move(document)) {}
    QStringList declaredFiles() const override { return qtStrings(document_.declaredFiles()); }
    bool isSingleBinaryRawAudio(int maximumTracks) const override { return document_.isSingleBinaryRawAudio(maximumTracks); }
    std::unique_ptr<IEmulationCueSheet> withSingleFile(const QString& declaredPath) const override {
        return std::make_unique<CueSheet>(document_.withSingleFile(declaredPath.toStdString()));
    }
    void write(std::ostream& stream, std::stop_token token) const override { document_.write(stream, token); }
private:
    MediaEngine::CueDocument document_;
};

class DriveMechanics final : public IEmulationDriveMechanics {
public:
    explicit DriveMechanics(std::shared_ptr<MediaEngine::DriveMechanics> source) : source_(std::move(source)) {}
    const std::shared_ptr<MediaEngine::DriveMechanics>& source() const { return source_; }
    bool hasPendingAudio() const override { return source_->hasPendingAudio(); }
    void configure(const QString& profile, double volumePercent, bool enabled) override {
        source_->configure(profile.isEmpty() ? std::nullopt : std::optional(profile.toStdString()), volumePercent, enabled);
    }
    void setTapeOperation(const QString& operation) override { source_->setTapeOperation(operation.toStdString()); }
    void insert() override { source_->insert(); }
    void eject() override { source_->eject(); }
    void setMotor(bool running) override { source_->setMotor(running); }
    void setActivity(bool reading, bool writing) override { source_->setActivity(reading, writing); }
    void seek(int cylinder, int head, int sector, qint64 logicalBlock) override {
        source_->seek({cylinder, head, sector, logicalBlock});
    }
    QVector<qint16> renderAudio(int sampleRate, int sampleCount) override {
        const auto samples = source_->renderAudio(sampleRate, sampleCount);
        return QVector<qint16>(samples.begin(), samples.end());
    }
    void reset() override { source_->reset(); }

private:
    std::shared_ptr<MediaEngine::DriveMechanics> source_;
};

class DiskImage final : public IEmulationDiskImage {
public:
    DiskImage(std::unique_ptr<MediaEngine::RuntimeDisk> disk, std::shared_ptr<IEmulationDriveMechanics> mechanics)
        : disk_(std::move(disk)),
          mechanics_(mechanics ? std::move(mechanics) : std::make_shared<DriveMechanics>(disk_->mechanics())) {}
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
        return diskSector(disk_->readSectorAsync(index, token).get());
    }
    void write(qint64 index, const QByteArray& bytes, std::optional<bool> deleted) override {
        disk_->writeSector(index, byteSpan(bytes), deleted);
    }
    void flush(std::stop_token token) override { disk_->flushAsync(token).get(); }
    void close() override { disk_->close(); }
    std::shared_ptr<IEmulationDriveMechanics> mechanics() const override { return mechanics_; }
    QMap<QString, QString> metadata() const override {
        return qtMap(disk_->metadata());
    }
    void updateMetadata(const QMap<QString, QString>& values) override {
        disk_->updateMetadata(stringMap(values));
    }
    void prepareBlockFormat(const DiskGeometry& geometry) override {
        disk_->prepareBlockFormat({geometry.cylinders, geometry.heads, geometry.sectorsPerTrack, geometry.sectorSize});
    }
    QList<qint64> trackSectors(int cylinder, int head) const override {
        const auto sectors = disk_->trackSectors(cylinder, head);
        return QList<qint64>(sectors.begin(), sectors.end());
    }
    DiskSectorDetails readDetails(qint64 index, std::stop_token token) const override {
        const auto sector = disk_->readSectorAsync(index, token).get();
        return {diskSector(sector), sector.diagnosticCode, byteArray(sector.recordedErrorCheckCode),
            byteArray(sector.recordedDataPrefix), sector.headerIntegrityValid, sector.dataIntegrityValid,
            sector.recordedSizeCode};
    }
    QList<DiskRevolution> readTrack(int cylinder, int head, const QString& encodingId,
        quint32 bitCellNanoseconds, quint32 indexTimeNanoseconds, const QMap<QString, int>& attributes) const override {
        const auto track = disk_->readTrack(cylinder, head, encodingId.toStdString(), bitCellNanoseconds,
            indexTimeNanoseconds, fluxAttributes(attributes));
        QList<DiskRevolution> result;
        for (const auto& revolution : track.revolutions()) {
            const auto intervals = revolution.intervalsNanoseconds();
            result.append(DiskRevolution{revolution.indexTimeNanoseconds(),
                QVector<quint32>(intervals.begin(), intervals.end())});
        }
        return result;
    }
    void writeTrackWindow(int cylinder, int head, const QString& encodingId, quint32 bitCellNanoseconds,
        quint32 indexTimeNanoseconds, quint32 startNanoseconds, quint32 durationNanoseconds,
        const QVector<quint32>& transitions, const QMap<QString, int>& attributes, bool includeEndingInstant) override {
        disk_->writeTrackWindow(cylinder, head, encodingId.toStdString(), bitCellNanoseconds, indexTimeNanoseconds,
            startNanoseconds, durationNanoseconds, {transitions.constData(), std::size_t(transitions.size())},
            fluxAttributes(attributes), includeEndingInstant);
    }
    void formatTrack(int cylinder, int head, const QList<DiskSectorDetails>& sectors,
        std::optional<int> dataRate, std::optional<bool> mfm) override {
        std::vector<MediaEngine::MediaDiskSector> converted;
        converted.reserve(std::size_t(sectors.size()));
        for (const auto& details : sectors) {
            const auto& sector = details.sector;
            const auto bytes = byteSpan(sector.bytes);
            const auto check = byteSpan(details.recordedErrorCheckCode);
            const auto prefix = byteSpan(details.recordedDataPrefix);
            converted.push_back({sector.index, {bytes.begin(), bytes.end()}, sector.cylinder, sector.head, sector.number,
                sector.integrityValid, details.diagnosticCode, {check.begin(), check.end()}, {prefix.begin(), prefix.end()},
                details.headerIntegrityValid, details.dataIntegrityValid, sector.deletedDataMark, details.recordedSizeCode});
        }
        disk_->formatTrack(cylinder, head, converted, dataRate, mfm);
    }

private:
    std::unique_ptr<MediaEngine::RuntimeDisk> disk_;
    std::shared_ptr<IEmulationDriveMechanics> mechanics_;
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
    return openDiskWithMechanics(path, writable, format, {}, token);
}
std::shared_ptr<IEmulationDriveMechanics> MediaPreparation::createDriveMechanics(const QString& device) const {
    return std::make_shared<DriveMechanics>(drives_.create(device.toStdString()));
}
std::shared_ptr<IEmulationDriveMechanics> MediaPreparation::createHostDriveMechanics(const QString& device) const {
    return std::make_shared<DriveMechanics>(drives_.create(device.toStdString(), true));
}
std::unique_ptr<IEmulationDiskImage> MediaPreparation::openDiskWithMechanics(const QString& path, bool writable,
        const QString& format, std::shared_ptr<IEmulationDriveMechanics> mechanics, std::stop_token token) const {
    const auto id = format.isEmpty() ? std::nullopt : std::optional(format.toStdString());
    const auto bridge = std::dynamic_pointer_cast<DriveMechanics>(mechanics);
    if (mechanics && !bridge)
        throw EmulationMessageException({MessageCategory::Media, MessageCode::MediaUnsupported});
    auto disk = MediaEngine::RuntimeDisk::openAsync(mediaPath(path), writable, id, token,
        bridge ? bridge->source() : nullptr).get();
    return std::make_unique<DiskImage>(std::move(disk), std::move(mechanics));
}

std::unique_ptr<IEmulationOpticalDisc> MediaPreparation::openOptical(const QString& path, std::stop_token token) const {
    return std::make_unique<OpticalDisc>(MediaEngine::OpticalDiscSession::openAsync(mediaPath(path), token, true).get());
}
void MediaPreparation::createBlankDisk(const QString& path, const QString& format, std::stop_token token) const {
    MediaEngine::createBlankDiskAsync(mediaPath(path), format.toStdString(), token).get();
}

TapeSignal MediaPreparation::readTape(const QString& path, std::stop_token token, const QString& machineId,
        const QMap<QString, QString>& parameters) const {
    const auto tape = MediaEngine::readTapeAsync(mediaPath(path), token, optionalString(machineId), stringMap(parameters)).get();
    const auto samples = tape.samples();
    TapeSignal result{QVector<qint16>(samples.begin(), samples.end()), tape.sampleRate(), {}, {}};
    if (const auto& positions = tape.blockPositions())
        result.blockPositions.emplace(positions->begin(), positions->end());
    if (const auto& blocks = tape.blocks()) {
        auto& list = result.blocks.emplace();
        list.reserve(qsizetype(blocks->size()));
        for (const auto& block : *blocks)
            list.append(TapeBlock{block.samplePosition(), byteArray(block.data()), block.integrityValid(),
                qtMap(block.metadata())});
    }
    return result;
}
void MediaPreparation::writeTape(const QString& path, const TapeSignal& tape, std::stop_token token,
        const QMap<QString, QString>& parameters) const {
    MediaEngine::writeTapeAsync(mediaPath(path),
        MediaEngine::MediaTapeSignal({tape.samples.begin(), tape.samples.end()}, tape.sampleRate),
        token, stringMap(parameters)).get();
}
QStringList MediaPreparation::tapeRecordingExtensions() const {
    return qtStrings(MediaEngine::tapeRecordingExtensions());
}
QStringList MediaPreparation::mechanicalSoundProfiles(MediaKind kind) const {
    return qtStrings(MediaEngine::MechanicalSoundService::profiles(mechanicalMediaKind(kind)));
}
QString MediaPreparation::mechanicalSoundProfileDisplayName(const QString& profile) const {
    return QString::fromStdString(MediaEngine::MechanicalSoundService::profileDisplayName(profile.toStdString()));
}
QVector<qint16> MediaPreparation::renderMechanicalSound(MediaKind kind, const QString& operation, int sampleRate,
        int sampleCount, qint64 offset, const QString& profile, double volumePercent) const {
    const auto samples = MediaEngine::MechanicalSoundService{}.render(mechanicalMediaKind(kind), operation.toStdString(),
        sampleRate, sampleCount, offset, optionalString(profile), volumePercent);
    return QVector<qint16>(samples.begin(), samples.end());
}
double MediaPreparation::mechanicalSoundDurationSeconds(MediaKind kind, const QString& operation, const QString& profile) const {
    return MediaEngine::MechanicalSoundService{}.duration(mechanicalMediaKind(kind), operation.toStdString(),
        optionalString(profile));
}
QVector<qint16> MediaPreparation::protectMechanicalAudioMix(const QVector<double>& samples) const {
    const auto mixed = MediaEngine::protectMechanicalAudioMix({samples.constData(), std::size_t(samples.size())});
    return QVector<qint16>(mixed.begin(), mixed.end());
}
QMap<QString, QString> MediaPreparation::mediaFileExtensions() const {
    return qtMap(MediaEngine::mediaFileExtensions());
}
OpticalSectorSizes MediaPreparation::opticalSectorSizes() const noexcept {
    return {MediaEngine::rawOpticalSectorSize(), MediaEngine::audioOpticalSectorSize(), MediaEngine::opticalSubchannelSize()};
}
void MediaPreparation::convertScp(const QString& source, const QString& destination, const QString& outputExtension,
        std::stop_token token) const {
    MediaEngine::convertScpAsync(mediaPath(source), mediaPath(destination), outputExtension.toStdString(), token).get();
}
std::unique_ptr<IEmulationCueSheet> MediaPreparation::readCueSheet(const QStringList& lines, std::stop_token token) const {
    std::vector<std::string> source;
    source.reserve(std::size_t(lines.size()));
    for (const auto& line : lines) source.push_back(line.toStdString());
    return std::make_unique<CueSheet>(MediaEngine::CueDocument::parse(source, token));
}
void MediaPreparation::prepareCueWaveFiles(const QString& source, const QString& destination, CueStageFile stageFile,
        int copyBufferBytes, std::stop_token token, CueUnsupported unsupported) const {
    MediaEngine::CueStageFile stage;
    if (stageFile)
        stage = [stageFile = std::move(stageFile)](const std::filesystem::path& path) {
            return mediaPath(stageFile(QString::fromStdU16String(path.u16string())));
        };
    MediaEngine::prepareCueWaveFilesAsync(mediaPath(source), mediaPath(destination), std::move(stage),
        copyBufferBytes, token, std::move(unsupported)).get();
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
    for (const auto& suffix : tapeRecordingExtensions()) {
        const auto& extension = suffix;
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
