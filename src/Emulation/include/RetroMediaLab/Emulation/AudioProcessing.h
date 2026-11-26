#pragma once

#include "EmulationExport.h"
#include "Sdk.h"
#include <array>
#include <span>
#include <vector>

namespace RetroMediaLab::Emulation {
class IAudioOutput;

namespace AudioProcessing {
inline constexpr int DefaultLatencyMs = 50;
inline constexpr std::array LatencyChoicesMs{20, 30, 40, DefaultLatencyMs, 75, 100, 150, 200};
inline constexpr float FullVolume = 1.0f;

RETROMEDIALAB_EMULATION_API bool hasInvalidConfiguredLatency(QStringView value);
RETROMEDIALAB_EMULATION_API bool hasInvalidConfiguredVolume(QStringView value, int minimum, int maximum);
RETROMEDIALAB_EMULATION_API QString resolveRuntimeOutput(const QMap<QString, QString>& options, const QString& key,
                                                        const QString& defaultOutput);
RETROMEDIALAB_EMULATION_API int resolveRuntimeLatency(const QMap<QString, QString>& options, const QString& key);
RETROMEDIALAB_EMULATION_API int maximumBufferedFrames(int sampleRate) noexcept;
RETROMEDIALAB_EMULATION_API AudioChunk retainNewestFrames(AudioChunk chunk, int maximumFrames);
RETROMEDIALAB_EMULATION_API QList<qint16> copyNativeBatch(const qint16* samples, std::size_t frameCount,
                                                         unsigned channelCount = 2);
RETROMEDIALAB_EMULATION_API float normalizeVolume(float volume) noexcept;

// At full volume the returned view borrows samples, otherwise it borrows buffer.
RETROMEDIALAB_EMULATION_API std::span<const qint16> applyVolume(std::span<const qint16> samples, float volume,
                                                               std::vector<qint16>& buffer);
RETROMEDIALAB_EMULATION_API void writeVolume(IAudioOutput* output, float volume, std::span<const qint16> samples);
}
}
