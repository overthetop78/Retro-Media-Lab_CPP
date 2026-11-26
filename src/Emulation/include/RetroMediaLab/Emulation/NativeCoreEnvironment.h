#pragma once

#include "EmulationKey.h"
#include "HostProtocol.h"
#include "NativeCoreFrames.h"
#include "NativeCoreLibrary.h"
#include "NativeCoreOptions.h"
#include "NativeCoreFileSystem.h"
#include "NativeCoreDiskControl.h"
#include <QLocale>
#include <memory>

namespace RetroMediaLab::Emulation {
inline constexpr unsigned NativeCoreExperimentalCommandFlag = 0x10000;

enum class NativeCoreEnvironmentCommand : unsigned {
    SetRotation = 1, GetOverscan = 2, GetCanDuplicateFrames = 3, SetMessage = 6, Shutdown = 7, SetPerformanceLevel = 8,
    GetSystemDirectory = 9, SetPixelFormat = 10, SetInputDescriptors = 11, SetKeyboardCallback = 12,
    SetDiskControl = 13, SetHardwareRender = 14,
    GetVariable = 15, SetVariables = 16, GetVariableUpdate = 17,
    SetSupportNoGame = 18, GetRumbleInterface = 23, GetInputDeviceCapabilities = 24,
    GetSensorInterface = 25 | NativeCoreExperimentalCommandFlag, GetLogInterface = 27, GetContentDirectory = 30,
    GetSaveDirectory = 31, SetSystemAvInfo = 32, SetControllerInfo = 35,
    SetMemoryMaps = 36 | NativeCoreExperimentalCommandFlag, SetGeometry = 37,
    GetLanguage = 39, SetSupportAchievements = 42 | NativeCoreExperimentalCommandFlag,
    GetVfsInterface = 45 | NativeCoreExperimentalCommandFlag,
    GetLedInterface = 46 | NativeCoreExperimentalCommandFlag,
    GetFastForwarding = 49 | NativeCoreExperimentalCommandFlag,
    GetInputBitmasks = 51 | NativeCoreExperimentalCommandFlag,
    GetCoreOptionsVersion = 52, SetCoreOptions = 53,
    SetCoreOptionsInternational = 54, SetCoreOptionsDisplay = 55, GetDiskControlVersion = 57,
    SetDiskControlExtended = 58, GetMessageInterfaceVersion = 59, SetMessageExtended = 60,
    SetAudioBufferStatusCallback = 62, SetMinimumAudioLatency = 63,
    SetCoreOptionsV2 = 67, SetCoreOptionsV2International = 68, SetCoreOptionsUpdateDisplayCallback = 69,
    SetVariable = 70,
};

struct NativeCoreMessage {
    const char* text;
    unsigned frames;
};

struct NativeCoreExtendedMessage {
    const char* text;
    unsigned durationMilliseconds, priority, level, target, type;
    std::int8_t progress;
};

struct NativeCoreMessageInfo {
    QString text;
    unsigned frames;
};

struct NativeCoreExtendedMessageInfo {
    QString text;
    unsigned durationMilliseconds, priority, level, target, type;
    std::int8_t progress;
};

struct NativeCoreSensorInterface {
    bool (*setState)(unsigned port, unsigned action, unsigned rate) = nullptr;
    float (*getInput)(unsigned port, unsigned id) = nullptr;
};

struct NativeCoreInputDescriptor {
    unsigned port, device, index, id;
    const char* description;
};

struct NativeCoreInputDescription {
    unsigned port, device, index, id;
    QString description;
};

struct NativeCoreControllerDevice {
    const char* description;
    unsigned id;
};

struct NativeCoreControllerInfo {
    const NativeCoreControllerDevice* types;
    unsigned count;
};

struct NativeCoreControllerDescription {
    QString description;
    unsigned id;
};

struct NativeCoreMemoryDescriptor {
    std::uint64_t flags;
    void* pointer;
    std::size_t offset, start, select, disconnect, length;
    const char* addressSpace;
};

struct NativeCoreMemoryMap {
    const NativeCoreMemoryDescriptor* descriptors;
    unsigned count;
};

struct NativeCoreMemoryDescription {
    std::uint64_t flags;
    void* pointer;
    std::size_t offset, start, select, disconnect, length;
    QString addressSpace;
};

struct NativeCoreCallbacks {
    NativeCoreExports::EnvironmentCallback environment;
    NativeCoreExports::VideoCallback video;
    NativeCoreExports::AudioSampleCallback audioSample;
    NativeCoreExports::AudioBatchCallback audioBatch;
    NativeCoreExports::InputPollCallback inputPoll;
    NativeCoreExports::InputStateCallback inputState;
};

RETROMEDIALAB_EMULATION_API void installNativeCoreCallbacks(const NativeCoreExports&, const NativeCoreCallbacks&);

struct NativeCoreDirectories {
    QString system;
    QString content;
    QString save;
    QString assets;
};

struct NativeCoreEnvironmentInterfaces {
    using Rumble = bool (*)(unsigned port, unsigned effect, quint16 strength);
    using Log = void (*)(int level, const char* message, ...);
    using Led = void (*)(int led, int state);
    Rumble rumble = nullptr;
    Log log = nullptr;
    const NativeCoreFileSystem* files = nullptr;
    NativeCoreDiskControl* disks = nullptr;
    Led led = nullptr;
    NativeCoreSensorInterface sensor;
    std::optional<std::uint64_t> inputCapabilities;
    bool inputBitmasks = false;
};

// The session owns these resources until the core has stopped calling back.
class RETROMEDIALAB_EMULATION_API NativeCoreEnvironment final {
public:
    using KeyboardCallback = void (*)(bool down, quint32 code, quint32 character, quint16 modifiers);
    using AudioBufferCallback = void (*)(bool active, unsigned occupancy, bool underrunLikely);

    NativeCoreEnvironment(NativeCoreVideoFrames&, NativeCoreAudioFrames&, const NativeCoreDirectories&,
        NativeCoreEnvironmentInterfaces interfaces = {}, QLocale uiLocale = {});
    NativeCoreEnvironment(NativeCoreVideoFrames&, NativeCoreAudioFrames&, const NativeCoreDirectories&,
        NativeCoreEnvironmentInterfaces interfaces, QLocale uiLocale, QHash<QString, QString> configuredOptions);
    NativeCoreEnvironment(const NativeCoreEnvironment&) = delete;
    NativeCoreEnvironment& operator=(const NativeCoreEnvironment&) = delete;

    bool dispatch(unsigned command, void* data) noexcept;
    void applyAvInfo(const NativeCoreAvInfo&);
    void publishKeyboard(const QSet<EmulationKey>& keys, const QHash<EmulationKey, quint32>& codes);
    void publishAudioStatus(const std::optional<HostProtocol::AudioBufferStatus>&) const;
    const QList<NativeCoreOption>& options() const noexcept;
    const QList<NativeCoreOptionCategoryInfo>& optionCategories() const noexcept;
    const QList<NativeCoreInputDescription>& inputDescriptions() const noexcept;
    const QList<QList<NativeCoreControllerDescription>>& controllerPorts() const noexcept;
    const QList<NativeCoreMemoryDescription>& memoryDescriptions() const noexcept;
    const QList<NativeCoreMessageInfo>& messages() const noexcept;
    const QList<NativeCoreExtendedMessageInfo>& extendedMessages() const noexcept;
    NativeCoreGeometry geometry() const noexcept;
    unsigned rotation() const noexcept;
    unsigned performanceLevel() const noexcept;
    void setOption(const QString& key, const QString& value);
    void recordDiagnostic(const char* utf8) noexcept;
    const QStringList& diagnostics() const noexcept;
    std::exception_ptr failure() const noexcept;
    void rethrowCallbackFailure() const;
    bool supportsNoGame() const noexcept;
    bool supportsAchievements() const noexcept;
    bool shutdownRequested() const noexcept;
    double framesPerSecond() const noexcept;
    int minimumAudioLatencyMilliseconds() const noexcept;

private:
    friend class NativeCoreSession;
    void clearBorrowedReferences() noexcept;
    void clearOutputs();
    NativeCoreVideoFrames& video_;
    NativeCoreAudioFrames& audio_;
    NativeCoreString systemDirectory_, contentDirectory_, saveDirectory_;
    NativeCoreEnvironmentInterfaces interfaces_;
    QLocale uiLocale_;
    NativeCoreLegacyOptions options_;
    QList<NativeCoreInputDescription> inputDescriptions_;
    QList<QList<NativeCoreControllerDescription>> controllerPorts_;
    QList<NativeCoreMemoryDescription> memoryDescriptions_;
    QList<NativeCoreMessageInfo> messages_;
    QList<NativeCoreExtendedMessageInfo> extendedMessages_;
    QSet<EmulationKey> previousKeys_;
    QStringList diagnostics_;
    std::exception_ptr failure_;
    KeyboardCallback keyboard_ = nullptr;
    AudioBufferCallback audioBufferStatus_ = nullptr;
    double framesPerSecond_ = 0;
    NativeCoreGeometry geometry_{};
    unsigned rotation_ = 0;
    unsigned performanceLevel_ = 0;
    int minimumAudioLatency_ = 0;
    bool supportsNoGame_ = false;
    bool supportsAchievements_ = false;
    bool shutdownRequested_ = false;
    std::unique_ptr<NativeCoreFileSystem> defaultFiles_;
};
}
