#pragma once

#include "NativeCoreContent.h"
#include <QLibrary>
#include <QStringList>
#include <cstdint>
#include <memory>
#include <stdexcept>

QT_BEGIN_NAMESPACE
class QTemporaryDir;
QT_END_NAMESPACE

namespace RetroMediaLab::Emulation {
struct NativeCoreSystemInfo {
    const char* libraryName;
    const char* libraryVersion;
    const char* validExtensions;
    bool needsFullPath;
    bool blockExtract;
};

struct NativeCoreGeometry {
    unsigned baseWidth;
    unsigned baseHeight;
    unsigned maximumWidth;
    unsigned maximumHeight;
    float aspectRatio;
};

struct NativeCoreTiming {
    double framesPerSecond;
    double sampleRate;
};

struct NativeCoreAvInfo {
    NativeCoreGeometry geometry;
    NativeCoreTiming timing;
};

// Borrowed entry points. Only use them while their NativeCoreLibrary is alive.
struct NativeCoreExports {
    using EnvironmentCallback = bool (*)(unsigned command, void* data);
    using VideoCallback = void (*)(const void* data, unsigned width, unsigned height, std::size_t pitch);
    using AudioSampleCallback = void (*)(std::int16_t left, std::int16_t right);
    using AudioBatchCallback = std::size_t (*)(const std::int16_t* data, std::size_t frames);
    using InputPollCallback = void (*)();
    using InputStateCallback = std::int16_t (*)(unsigned port, unsigned device, unsigned index, unsigned id);
    using SystemInfo = void (*)(NativeCoreSystemInfo*);
    using LoadGameSpecial = bool (*)(unsigned subsystem, const NativeCoreGameInfo*, std::size_t count);

    void (*setEnvironment)(EnvironmentCallback);
    void (*setVideoRefresh)(VideoCallback);
    void (*setAudioSample)(AudioSampleCallback);
    void (*setAudioSampleBatch)(AudioBatchCallback);
    void (*setInputPoll)(InputPollCallback);
    void (*setInputState)(InputStateCallback);
    void (*initialize)();
    void (*deinitialize)();
    SystemInfo getSystemInfo;
    void (*getSystemAvInfo)(NativeCoreAvInfo*);
    void (*setControllerPortDevice)(unsigned port, unsigned device);
    void (*reset)();
    void (*run)();
    bool (*loadGame)(const NativeCoreGameInfo*);
    void (*unloadGame)();
    unsigned (*getRegion)();
    void* (*getMemoryData)(unsigned id);
    std::size_t (*getMemorySize)(unsigned id);
    std::size_t (*getSerializedSize)();
    bool (*serialize)(void* data, std::size_t size);
    bool (*unserialize)(const void* data, std::size_t size);
    LoadGameSpecial loadGameSpecial;
};

struct NativeCoreMetadata {
    QString libraryName;
    QString libraryVersion;
    QStringList extensions;
    bool needsFullPath;
    bool blockExtract;
};

enum class NativeCoreLibraryFailure { RelativePath, MissingFile, LoadFailed, MissingExport, ApiVersion, Identity, StagingFailed };

class RETROMEDIALAB_EMULATION_API NativeCoreLibraryError final : public std::runtime_error {
public:
    NativeCoreLibraryError(NativeCoreLibraryFailure, QString detail);
    NativeCoreLibraryFailure failure() const noexcept;
    const QString& detail() const noexcept;

private:
    NativeCoreLibraryFailure failure_;
    QString detail_;
};

class RETROMEDIALAB_EMULATION_API NativeCoreLibrary final {
public:
    explicit NativeCoreLibrary(const QString& absolutePath);
    NativeCoreLibrary(const QString& absolutePath, const QString& isolationDirectory);
    ~NativeCoreLibrary();
    NativeCoreLibrary(const NativeCoreLibrary&) = delete;
    NativeCoreLibrary& operator=(const NativeCoreLibrary&) = delete;

    static void validatePath(const QString& absolutePath);
    QFunctionPointer resolve(const char* exportName);
    NativeCoreExports resolveExports(bool requireLoadGameSpecial);
    NativeCoreExports::LoadGameSpecial resolveSpecialContent();
    NativeCoreMetadata inspect(const NativeCoreExports&, unsigned expectedApiVersion, const QString& expectedName);

private:
    std::unique_ptr<QTemporaryDir> staging_;
    QLibrary library_;
};

RETROMEDIALAB_EMULATION_API QStringList parseNativeCoreExtensions(const char* extensions);
RETROMEDIALAB_EMULATION_API NativeCoreMetadata inspectNativeCoreLibrary(const QString& absolutePath,
    unsigned expectedApiVersion, const QString& expectedName, bool requireLoadGameSpecial);
}
