#pragma once
#include <RetroMediaLab/Emulation/Sdk.h>

namespace RetroMediaLab::Emulation {
struct HardDiskImageGeometry;
class MediaPreparation final : public IMediaPreparation {
public:
    QString detectFormat(const QString& path, std::stop_token) const override;
    std::unique_ptr<IEmulationDiskImage> openDisk(const QString& path, bool writable,
        const QString& format, std::stop_token) const override;
    std::unique_ptr<IEmulationOpticalDisc> openOptical(const QString& path, std::stop_token) const override;
    void createBlankDisk(const QString& path, const QString& format, std::stop_token) const override;
    QStringList tapeRecordingExtensions(const MediaSlot&) const;
    void createBlankTape(const QString& path, std::optional<TapePcmSettings>, std::stop_token) const;
    void createHardDisk(const QString& path, const QString& formatId, const HardDiskImageGeometry&,
        const QString& fileSystemId, bool preallocate, std::stop_token) const;
};
}
