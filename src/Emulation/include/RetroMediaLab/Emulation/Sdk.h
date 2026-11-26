#pragma once

#include <QByteArray>
#include <QJsonObject>
#include <QList>
#include <QSet>
#include <QStringList>
#include <QUuid>
#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <stop_token>
#include <utility>

namespace RetroMediaLab::Emulation {
inline constexpr auto HostApiMajor = 2;
inline constexpr auto HostApiMinor = 2;
inline constexpr auto FactorySymbol = "rml_emulation_factory_v2";

enum class MachineState { Created, Starting, Running, Paused, Stopping, Stopped, Faulted };
enum class HostProcessState { InProcess, NotStarted, Running, Exited, Faulted };
enum class PixelFormat { Rgb565, Xrgb8888, Rgb1555 };
enum class ScanMode { Progressive, EvenField, OddField };
enum class VideoStandard { Pal, Ntsc, Secam };
enum class LifecycleCommand { Start, Pause, Resume, SoftReset, HardReset, Stop };
enum class MediaKind { Floppy, HardDisk, OpticalDisc, Tape, Cartridge, MemoryCard };
enum class TapeCommand { Play, Stop, Rewind, FastForward, Record, Pause };
enum class TapeState { Empty, Stopped, Playing, Paused, Recording, EndOfTape };

struct TapeTransportStatus {
    TapeState state = TapeState::Empty;
    std::optional<TapeCommand> activeOperation;
    QSet<TapeCommand> availableCommands;
};

struct VideoFrame {
    QByteArray pixels;
    int width = 0;
    int height = 0;
    int pitch = 0;
    PixelFormat pixelFormat = PixelFormat::Xrgb8888;
    float aspectRatio = 0;
    quint64 sequence = 0;
    std::chrono::nanoseconds timestamp{};
    ScanMode scanMode = ScanMode::Progressive;
    std::optional<VideoStandard> signalStandard;
    double signalFieldRate = 0;
    int signalFieldHeight = 0;
};

enum AudioChannel : quint32 {
    FrontLeft = 0x001, FrontRight = 0x002, FrontCenter = 0x004, LowFrequency = 0x008,
    BackLeft = 0x010, BackRight = 0x020, SideLeft = 0x200, SideRight = 0x400,
    Stereo = FrontLeft | FrontRight,
    Surround71 = Stereo | FrontCenter | LowFrequency | BackLeft | BackRight | SideLeft | SideRight
};

struct AudioChunk {
    QList<qint16> interleavedSamples;
    int sampleRate = 0;
    int frameCount = 0;
    quint64 sequence = 0;
    std::chrono::nanoseconds timestamp{};
    quint32 channelLayout = Stereo;
};

struct AudioOutputBufferStatus {
    qint32 sampleRate = 0;
    qint32 bufferedFrames = 0;
    qint32 capacityFrames = 0;
    bool active = false;
};

class IMachineAudioBufferFeedback {
public:
    virtual ~IMachineAudioBufferFeedback() = default;
    virtual int minimumAudioLatencyMs() const noexcept { return 0; }
    virtual void setAudioBufferStatus(std::optional<AudioOutputBufferStatus>) noexcept = 0;
};

struct InputSnapshot {
    // EmulationKey numeric identities, shared with the hosted-core protocol.
    QSet<int> keys;
    QHash<int, char32_t> hostCharacters;
    // Relative movement in screen pixels while the pointer is captured.
    int pointerX = 0, pointerY = 0, wheel = 0;
    quint32 pointerButtons = 0;
    bool pointerCaptured = false;
    struct Controller {
        quint64 buttons = 0;
        // Left X/Y, right X/Y, left trigger, right trigger, in that order.
        QList<float> axes;
        bool operator==(const Controller&) const = default;
    };
    QList<Controller> controllers;
    int horizontalWheel = 0;
};

struct MediaSlot {
    QString id;
    QString displayResourceKey;
    MediaKind kind = MediaKind::Floppy;
    QStringList acceptedExtensions;
    QHash<QString, QString> settings;
    bool operator==(const MediaSlot&) const = default;
};

struct MountedMedia {
    MediaSlot slot;
    QString path;
    bool readOnly = true;
    QHash<QString, QString> parameters;
};

struct TapePcmSettings {
    static constexpr int DefaultSampleRate = 48000;
    int sampleRate = DefaultSampleRate;
    int channels = 1;
    int bitsPerSample = 16;
    bool signedSamples = true;
    bool littleEndian = true;

    QHash<QString, QString> parameters() const {
        return {{QStringLiteral("sampleRate"), QString::number(sampleRate)},
            {QStringLiteral("channels"), QString::number(channels)},
            {QStringLiteral("bitsPerSample"), QString::number(bitsPerSample)},
            {QStringLiteral("signed"), signedSamples ? QStringLiteral("true") : QStringLiteral("false")},
            {QStringLiteral("littleEndian"), littleEndian ? QStringLiteral("true") : QStringLiteral("false")}};
    }
};

struct DiskStatus {
    QStringList images;
    int currentIndex = -1;
    bool ejected = true;
};

struct Configuration {
    QUuid id;
    QString machineId;
    QString name;
    QJsonObject values;
};

enum class SettingsEditor { Text, Multiline, Toggle, Choice, File, Folder, Information, Slider, FirmwareFile };

struct SettingsChoice {
    QString id;
    QString displayResourceKey;
    QString invariantDisplayValue;
};

struct SettingsField {
    QString id;
    QString labelResourceKey;
    SettingsEditor editor = SettingsEditor::Text;
    QString value;
    QList<SettingsChoice> choices;
    bool enabled = true;
    bool visible = true;
    QString explanationResourceKey;
    QString enabledValue = QStringLiteral("true");
    QString disabledValue = QStringLiteral("false");
};

struct ModuleAudioOutputSelection {
    QString fieldId;
    QString deviceId;
    int latencyMs = 0;
    float volume = 1.0f;
};

class IModuleAudioOutputSettings {
public:
    virtual ~IModuleAudioOutputSettings() = default;
    // Device IDs come from the host choices. An empty ID selects the system default.
    virtual ModuleAudioOutputSelection audioOutputSelection(const Configuration&) const = 0;
};

struct SettingsBlock {
    QString id;
    QString tabResourceKey;
    QString titleResourceKey;
    QList<SettingsField> fields;
    bool visible = true;
};

struct MachineDefinition {
    QString id;
    QString displayResourceKey;
    QString imagePath;
};

struct ModuleContext {
    QString libraryDirectory;
    QString configurationDirectory;
};

struct DiskGeometry {
    qint64 cylinders = 0, sectorCount = 0;
    int heads = 0, sectorsPerTrack = 0, sectorSize = 0;
};

struct DiskSector {
    qint64 index = 0;
    QByteArray bytes;
    int cylinder = 0, head = 0, number = 0;
    std::optional<bool> integrityValid;
    std::optional<bool> deletedDataMark;
};

class IEmulationDiskImage {
public:
    virtual ~IEmulationDiskImage() = default;
    virtual QString format() const = 0;
    virtual DiskGeometry geometry() const = 0;
    virtual bool readOnly() const = 0;
    virtual bool hasChanges() const = 0;
    virtual std::optional<qint64> findSector(int cylinder, int head, int number) const = 0;
    virtual DiskSector read(qint64 index, std::stop_token) const = 0;
    virtual void write(qint64 index, const QByteArray&, std::optional<bool> deleted) = 0;
    virtual void flush(std::stop_token) = 0;
    virtual void close() = 0;
};

struct OpticalTrack {
    int number = 0;
    qint64 firstSector = 0, sectorCount = 0;
    int storedSectorSize = 0, userDataOffset = 0, userDataSize = 0;
    bool audio = false, subchannels = false;
};

class IEmulationOpticalDisc {
public:
    virtual ~IEmulationOpticalDisc() = default;
    virtual QString format() const = 0;
    virtual QList<OpticalTrack> tracks() const = 0;
    virtual QByteArray readStoredSector(int trackIndex, qint64 relativeSector, std::stop_token) const = 0;
    virtual QByteArray readSubchannel(int trackIndex, qint64 relativeSector, std::stop_token) const = 0;
};

class IMediaPreparation {
public:
    virtual ~IMediaPreparation() = default;
    virtual QString detectFormat(const QString& path, std::stop_token) const = 0;
    virtual std::unique_ptr<IEmulationDiskImage> openDisk(const QString& path, bool writable,
        const QString& format, std::stop_token) const = 0;
    virtual std::unique_ptr<IEmulationOpticalDisc> openOptical(const QString& path, std::stop_token) const = 0;
    virtual void createBlankDisk(const QString& path, const QString& format, std::stop_token) const = 0;
};

struct RuntimeServices {
    const IMediaPreparation& media;
    QString sessionDirectory;
    QString stateDirectory;
    QString convertedMediaDirectory;
    QString executablePath;
};

class IEmulatedMachine {
public:
    virtual ~IEmulatedMachine() = default;
    virtual MachineState state() const = 0;
    virtual void execute(LifecycleCommand, std::stop_token) = 0;
    virtual std::shared_ptr<const VideoFrame> latestFrame() const = 0;
    virtual double framesPerSecond() const = 0;
    virtual void setInput(const InputSnapshot&) = 0;
    virtual std::optional<AudioChunk> takeAudio() { return {}; }
    virtual QList<MediaSlot> mediaSlots() const { return {}; }
    virtual QList<MountedMedia> mountedMedia() const { return {}; }
    virtual DiskStatus diskStatus(std::stop_token) const { return {}; }
    virtual bool insert(const MountedMedia&, std::stop_token) { return false; }
    virtual bool eject(const MediaSlot&, std::stop_token) { return false; }
    virtual bool selectDisk(const MediaSlot&, int, std::stop_token) { return false; }
    virtual bool hasUnsavedChanges(const MediaSlot&, std::stop_token) const { return false; }
    virtual bool saveChanges(const MediaSlot&, std::stop_token) { return false; }
    virtual bool tape(TapeCommand, std::stop_token) { return false; }
    virtual bool supportsSavedStates() const { return false; }
    virtual bool saveState(const QString&, std::stop_token) { return false; }
    virtual bool loadState(const QString&, std::stop_token) { return false; }
    virtual TapeTransportStatus tapeTransportStatus() const { return {}; }
};

struct CoreRuntimeStatus {
    std::optional<std::uint64_t> processorClockHz;
    bool supportsSaveStates = false;
    std::optional<VideoStandard> region;
    int bufferedAudioFrames = 0;
    quint64 audioOverrunCount = 0;
    quint64 audioUnderrunCount = 0;
    HostProcessState hostProcessState = HostProcessState::InProcess;
    std::optional<qint64> hostProcessId;
};

// An adapter publishes a snapshot that may be read by the machine's owner.
class IMachineCoreRuntimeStatus {
public:
    virtual ~IMachineCoreRuntimeStatus() = default;
    virtual CoreRuntimeStatus coreRuntimeStatus() const = 0;
};

class IEmulationModule {
public:
    virtual ~IEmulationModule() = default;
    virtual QString id() const = 0;
    virtual QString displayResourceKey() const = 0;
    virtual QString brandImagePath() const { return {}; }
    virtual QList<MachineDefinition> machines() const = 0;
    virtual QString translate(const QString& key, const QString& language) const = 0;
    virtual Configuration createConfiguration(const QString& machineId) = 0;
    virtual QList<SettingsBlock> describe(const QString&, const Configuration*) const { return {}; }
    virtual Configuration applySettings(Configuration configuration, const QJsonObject& values) {
        for (auto change = values.constBegin(); change != values.constEnd(); ++change)
            configuration.values.insert(change.key(), change.value());
        return configuration;
    }
    virtual QList<Configuration> loadConfigurations(std::stop_token) = 0;
    virtual void saveConfiguration(const Configuration&, std::stop_token) = 0;
    virtual void deleteConfiguration(const QUuid&, std::stop_token) = 0;
    virtual std::unique_ptr<IEmulatedMachine> createRuntime(const Configuration&, const RuntimeServices&, std::stop_token) = 0;
};

struct RuntimeOption {
    QString id;
    QString currentValue;
    bool requiresRestart = false;
};

class IModuleRuntimeOptions {
public:
    virtual ~IModuleRuntimeOptions() = default;
    virtual QHash<QString, QString> runtimeOptions(const Configuration&) const = 0;
};

struct RawControllerState {
    QString deviceId;
    // Raw GameInput values keyed by zero-based Axis, Button or Switch names.
    QHash<QString, float> controls;
    bool operator==(const RawControllerState&) const = default;
};

class IMachineRawControllerInput {
public:
    virtual ~IMachineRawControllerInput() = default;
    virtual void setRawControllerInput(const QList<RawControllerState>&) = 0;
};

enum class ControllerMotor { LowFrequency, HighFrequency, LeftTrigger, RightTrigger };

struct ControllerOutput {
    QString deviceId;
    ControllerMotor motor;
    float strength;
};

class IMachineControllerFeedback {
public:
    virtual ~IMachineControllerFeedback() = default;
    virtual QList<ControllerOutput> controllerOutputs() const = 0;
};

class IMachineMediaActivity {
public:
    virtual ~IMachineMediaActivity() = default;
    // Each pair identifies the kind and id of an active slot.
    virtual QSet<std::pair<MediaKind, QString>> activeMediaSlots() const = 0;
};

class IMachinePointerCapture {
public:
    virtual ~IMachinePointerCapture() = default;
    virtual bool supportsPointerCapture() const = 0;
    virtual bool capturePointerOnClick() const = 0;
};

// Optional runtime capability. A true mode denotes the emulated pointer.
class IMachineControllerPointerSwitch {
public:
    virtual ~IMachineControllerPointerSwitch() = default;
    virtual bool supportsControllerPointerSwitch() const = 0;
    virtual bool controllerPointerMode() const = 0;
    virtual bool switchControllerPointer(std::stop_token) = 0;
};

class IMachineRuntimeOptions {
public:
    virtual ~IMachineRuntimeOptions() = default;
    virtual QList<RuntimeOption> availableOptions() const = 0;
    virtual void setOption(const QString& id, const QString& value, std::stop_token) = 0;
};

class IModuleMountedMedia {
public:
    virtual ~IModuleMountedMedia() = default;
    virtual Configuration applyMountedMedia(Configuration, const QList<MountedMedia>&) = 0;
    virtual std::unique_ptr<IEmulatedMachine> createRuntimeWithMedia(const Configuration&, const RuntimeServices&,
        const QList<MountedMedia>&, std::stop_token) = 0;
};

struct SettingsNumericRange {
    double minimum = 0;
    double maximum = 0;
    double step = 1;
    QString unitSuffix;
    QString unitResourceKey;
    std::optional<QString> automaticValue;
    bool useChoiceLabels = false;
    bool vertical = false;
};

class IModuleNumericSettings {
public:
    virtual ~IModuleNumericSettings() = default;
    virtual QHash<QString, SettingsNumericRange> numericSettings(const Configuration&) const = 0;
};

enum class VideoSettingsSection { Display, Image, Effects };

class IModuleVideoSettingsSections {
public:
    virtual ~IModuleVideoSettingsSections() = default;
    // Keys are video setting field IDs. Unlisted fields belong to Display.
    virtual QHash<QString, VideoSettingsSection> videoSettingsSections(const Configuration&) const = 0;
};

struct EditableNumericSetting {
    std::optional<double> fineStep;
};

class IModuleEditableNumericSettings {
public:
    virtual ~IModuleEditableNumericSettings() = default;
    virtual QHash<QString, EditableNumericSetting> editableNumericSettings(const Configuration&) const = 0;
};

enum class SettingsRuleCategory { MutuallyExclusive, VisibleWhenSourceDiffers };

struct SettingsRule {
    SettingsRuleCategory category;
    QString sourceFieldId;
    QString targetFieldId;
    QString comparedValue;
};

struct SettingsBlockDependency {
    QString fieldId;
    QString visibleValue;
};

struct SettingsDependencies {
    QList<SettingsRule> rules;
    QHash<QString, SettingsBlockDependency> blocks;
    QSet<QString> hiddenTabs;
};

class IModuleSettingsDependencies {
public:
    virtual ~IModuleSettingsDependencies() = default;
    virtual SettingsDependencies settingsDependencies(const Configuration&) const = 0;
};

struct ModuleFactory {
    int apiMajor;
    int apiMinor;
    IEmulationModule* (*create)(const ModuleContext&);
    void (*destroy)(IEmulationModule*) noexcept;
};

using ModuleFactoryEntry = const ModuleFactory* (*)() noexcept;
}
