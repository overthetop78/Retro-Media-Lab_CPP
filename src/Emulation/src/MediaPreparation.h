#pragma once
#include <RetroMediaLab/Emulation/Sdk.h>
#include <RetroMediaLab/MediaEngine/DriveService.h>
#include "AudioOutput.h"

namespace RetroMediaLab::Emulation {
struct HardDiskImageGeometry;
class MediaPreparation final : public IMediaPreparation {
public:
    MediaPreparation() : mechanicalAudio_(drives_) {}
    QString detectFormat(const QString& path, std::stop_token) const override;
    std::unique_ptr<IEmulationDiskImage> openDisk(const QString& path, bool writable,
        const QString& format, std::stop_token) const override;
    std::shared_ptr<IEmulationDriveMechanics> createDriveMechanics(const QString& device) const override;
    std::shared_ptr<IEmulationDriveMechanics> createHostDriveMechanics(const QString& device) const override;
    std::unique_ptr<IEmulationDiskImage> openDiskWithMechanics(const QString& path, bool writable,
        const QString& format, std::shared_ptr<IEmulationDriveMechanics>, std::stop_token) const override;
    std::unique_ptr<IEmulationOpticalDisc> openOptical(const QString& path, std::stop_token) const override;
    void createBlankDisk(const QString& path, const QString& format, std::stop_token) const override;
    TapeSignal readTape(const QString& path, std::stop_token, const QString& machineId,
        const QMap<QString, QString>& parameters) const override;
    void writeTape(const QString& path, const TapeSignal&, std::stop_token,
        const QMap<QString, QString>& parameters) const override;
    QStringList tapeRecordingExtensions() const override;
    QStringList mechanicalSoundProfiles(MediaKind) const override;
    QString mechanicalSoundProfileDisplayName(const QString& profile) const override;
    QVector<qint16> renderMechanicalSound(MediaKind, const QString& operation, int sampleRate, int sampleCount,
        qint64 offset, const QString& profile, double volumePercent) const override;
    double mechanicalSoundDurationSeconds(MediaKind, const QString& operation, const QString& profile) const override;
    QVector<qint16> protectMechanicalAudioMix(const QVector<double>& samples) const override;
    QMap<QString, QString> mediaFileExtensions() const override;
    OpticalSectorSizes opticalSectorSizes() const noexcept override;
    void convertScp(const QString& source, const QString& destination, const QString& outputExtension,
        std::stop_token) const override;
    std::unique_ptr<IEmulationCueSheet> readCueSheet(const QStringList& lines, std::stop_token) const override;
    void prepareCueWaveFiles(const QString& source, const QString& destination, CueStageFile,
        int copyBufferBytes, std::stop_token, CueUnsupported) const override;
    QStringList tapeRecordingExtensions(const MediaSlot&) const;
    void createBlankTape(const QString& path, std::optional<TapePcmSettings>, std::stop_token) const;
    void createHardDisk(const QString& path, const QString& formatId, const HardDiskImageGeometry&,
        const QString& fileSystemId, bool preallocate, std::stop_token) const override;

private:
    MediaEngine::MediaDriveService drives_;
    MechanicalAudioOutput mechanicalAudio_;
};
}
