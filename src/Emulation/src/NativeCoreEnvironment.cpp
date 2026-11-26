#include <RetroMediaLab/Emulation/NativeCoreEnvironment.h>
#include <RetroMediaLab/Emulation/InputProcessing.h>
#include <QDir>
#include <QFileInfo>
#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace RetroMediaLab::Emulation {
namespace {
constexpr auto ErrorKey = "Emulation.Error.EmulatorRejected";
constexpr int MaximumAudioLatencyMilliseconds = 512;
constexpr int FullBufferPercent = 100;
constexpr unsigned EnglishLanguage = 0;
constexpr unsigned FrenchLanguage = 2;
constexpr unsigned MaximumInputDescriptors = 4096;
constexpr unsigned MaximumControllerPorts = 32;
constexpr unsigned MaximumControllerTypes = 256;
constexpr unsigned MaximumMemoryDescriptors = 4096;
constexpr unsigned LastRotation = 3;
constexpr unsigned MessageInterfaceVersion = 1;

QList<NativeCoreInputDescription> copyInputs(const NativeCoreInputDescriptor* data) {
    QList<NativeCoreInputDescription> inputs;
    if (!data) return inputs;
    for (unsigned index = 0; index < MaximumInputDescriptors && data[index].description; ++index) {
        const auto& item = data[index];
        inputs.append({item.port, item.device, item.index, item.id, QString::fromUtf8(item.description)});
    }
    return inputs;
}

QList<QList<NativeCoreControllerDescription>> copyPorts(const NativeCoreControllerInfo* data) {
    QList<QList<NativeCoreControllerDescription>> ports;
    if (!data) return ports;
    for (unsigned index = 0; index < MaximumControllerPorts && data[index].types && data[index].count; ++index) {
        const auto& port = data[index];
        const auto count = std::min(port.count, MaximumControllerTypes);
        QList<NativeCoreControllerDescription> types;
        types.reserve(count);
        for (unsigned device = 0; device < count; ++device) {
            const auto& type = port.types[device];
            types.append({type.description ? QString::fromUtf8(type.description) : QStringLiteral(""), type.id});
        }
        ports.append(std::move(types));
    }
    return ports;
}

QList<NativeCoreMemoryDescription> copyMemory(const NativeCoreMemoryMap* map) {
    QList<NativeCoreMemoryDescription> memory;
    if (!map || !map->descriptors) return memory;
    const auto count = std::min(map->count, MaximumMemoryDescriptors);
    memory.reserve(count);
    for (unsigned index = 0; index < count; ++index) {
        const auto& item = map->descriptors[index];
        memory.append({item.flags, item.pointer, item.offset, item.start, item.select, item.disconnect,
            item.length, QString::fromUtf8(item.addressSpace)});
    }
    return memory;
}

enum class NativePixelFormat : unsigned { Rgb1555 = 0, Xrgb8888 = 1, Rgb565 = 2 };

QString ensureDirectory(const QString& path) {
    if (path.isEmpty() || !QDir().mkpath(path)) throw std::runtime_error(ErrorKey);
    return QDir::toNativeSeparators(QDir::cleanPath(QFileInfo(path).absoluteFilePath()));
}

int sampleRate(double value) {
    if (!std::isfinite(value)) throw std::out_of_range(ErrorKey);
    const double lower = std::floor(value);
    const double midpoint = 0.5;
    const double rounded = value - lower == midpoint
        ? (std::fmod(lower, 2.0) == 0.0 ? lower : lower + 1.0)
        : std::round(value);
    if (rounded < std::numeric_limits<int>::min() || rounded > std::numeric_limits<int>::max())
        throw std::out_of_range(ErrorKey);
    return static_cast<int>(rounded);
}
}

void installNativeCoreCallbacks(const NativeCoreExports& exports, const NativeCoreCallbacks& callbacks) {
    exports.setEnvironment(callbacks.environment);
    exports.setVideoRefresh(callbacks.video);
    exports.setAudioSample(callbacks.audioSample);
    exports.setAudioSampleBatch(callbacks.audioBatch);
    exports.setInputPoll(callbacks.inputPoll);
    exports.setInputState(callbacks.inputState);
}

NativeCoreEnvironment::NativeCoreEnvironment(NativeCoreVideoFrames& video, NativeCoreAudioFrames& audio,
    const NativeCoreDirectories& dirs, NativeCoreEnvironmentInterfaces interfaces, QLocale uiLocale)
    : NativeCoreEnvironment(video, audio, dirs, interfaces, std::move(uiLocale), {}) {}

NativeCoreEnvironment::NativeCoreEnvironment(NativeCoreVideoFrames& video, NativeCoreAudioFrames& audio,
    const NativeCoreDirectories& dirs, NativeCoreEnvironmentInterfaces interfaces, QLocale uiLocale,
    QHash<QString, QString> configuredOptions)
    : video_(video), audio_(audio), systemDirectory_(ensureDirectory(dirs.system)),
      contentDirectory_(ensureDirectory(dirs.content)), saveDirectory_(ensureDirectory(dirs.save)),
      interfaces_(interfaces), uiLocale_(std::move(uiLocale)), options_(std::move(configuredOptions)) {
    ensureDirectory(dirs.assets);
}

void NativeCoreEnvironment::applyAvInfo(const NativeCoreAvInfo& info) {
    framesPerSecond_ = info.timing.framesPerSecond;
    audio_.setSampleRate(sampleRate(info.timing.sampleRate));
    geometry_ = info.geometry;
    video_.applyGeometry(geometry_);
}

bool NativeCoreEnvironment::dispatch(unsigned command, void* data) noexcept {
    using Command = NativeCoreEnvironmentCommand;
    const auto id = static_cast<Command>(command);
    if (id == Command::Shutdown) {
        shutdownRequested_ = true;
        return true;
    }
    if (id == Command::SetAudioBufferStatusCallback) {
        audioBufferStatus_ = data ? *static_cast<const AudioBufferCallback*>(data) : nullptr;
        return true;
    }
    if (id == Command::SetMinimumAudioLatency) {
        minimumAudioLatency_ = data ? std::clamp(*static_cast<const int*>(data), 0, MaximumAudioLatencyMilliseconds) : 0;
        return true;
    }
    if (id == Command::GetInputBitmasks) {
        if (data) *static_cast<bool*>(data) = interfaces_.inputBitmasks;
        return interfaces_.inputBitmasks;
    }
    try {
        switch (id) {
        case Command::SetInputDescriptors:
            inputDescriptions_ = copyInputs(static_cast<const NativeCoreInputDescriptor*>(data));
            return data != nullptr;
        case Command::SetControllerInfo:
            controllerPorts_ = copyPorts(static_cast<const NativeCoreControllerInfo*>(data));
            return data != nullptr;
        case Command::SetMemoryMaps:
            memoryDescriptions_ = copyMemory(static_cast<const NativeCoreMemoryMap*>(data));
            return data != nullptr;
        case Command::SetSupportAchievements:
            supportsAchievements_ = data && *static_cast<const unsigned char*>(data) != 0;
            return data != nullptr;
        default: break;
        }
        if (!data && id != Command::GetVariable) return false;
        switch (id) {
        case Command::SetRotation: {
            const auto rotation = *static_cast<const unsigned*>(data);
            if (rotation > LastRotation) return false;
            rotation_ = rotation;
            return true;
        }
        case Command::GetOverscan:
        case Command::GetFastForwarding: *static_cast<bool*>(data) = false; return true;
        case Command::GetSystemDirectory: *static_cast<const char**>(data) = systemDirectory_.pointer(); return true;
        case Command::GetContentDirectory: *static_cast<const char**>(data) = contentDirectory_.pointer(); return true;
        case Command::GetSaveDirectory: *static_cast<const char**>(data) = saveDirectory_.pointer(); return true;
        case Command::GetCanDuplicateFrames: *static_cast<bool*>(data) = true; return true;
        case Command::GetLanguage:
            *static_cast<unsigned*>(data) = uiLocale_.language() == QLocale::French ? FrenchLanguage : EnglishLanguage;
            return true;
        case Command::GetVariable: return options_.returnValue(static_cast<NativeCoreVariable*>(data));
        case Command::GetVfsInterface: {
            auto* request = static_cast<NativeCoreVfsRequest*>(data);
            if (interfaces_.files) return interfaces_.files->provide(request);
            if (request->requiredInterfaceVersion > NativeCoreFileSystem::InterfaceVersion) return false;
            if (!defaultFiles_) defaultFiles_ = std::make_unique<NativeCoreFileSystem>();
            return defaultFiles_->provide(request);
        }
        case Command::GetDiskControlVersion:
            if (!interfaces_.disks) return false;
            *static_cast<unsigned*>(data) = NativeCoreDiskControlVersion;
            return true;
        case Command::SetDiskControl:
            if (!interfaces_.disks) return false;
            interfaces_.disks->capture(*static_cast<const NativeCoreDiskCallbacks*>(data));
            return true;
        case Command::SetDiskControlExtended:
            if (!interfaces_.disks) return false;
            interfaces_.disks->capture(*static_cast<const NativeCoreDiskExtendedCallbacks*>(data));
            return true;
        case Command::SetVariables: return options_.registerVariables(static_cast<const NativeCoreVariable*>(data));
        case Command::GetVariableUpdate: return options_.returnUpdated(static_cast<bool*>(data));
        case Command::GetCoreOptionsVersion:
            *static_cast<unsigned*>(data) = NativeCoreOptionsVersion;
            return true;
        case Command::SetCoreOptions:
            return options_.registerDefinitions(static_cast<const NativeCoreOptionDefinitionV1*>(data));
        case Command::SetCoreOptionsInternational: {
            const auto& tables = *static_cast<const NativeCoreInternationalOptions<NativeCoreOptionDefinitionV1>*>(data);
            return options_.registerDefinitions(tables.english, tables.local);
        }
        case Command::SetCoreOptionsV2:
            return options_.registerDefinitions(static_cast<const NativeCoreOptionTableV2*>(data));
        case Command::SetCoreOptionsV2International: {
            const auto& tables = *static_cast<const NativeCoreInternationalOptions<NativeCoreOptionTableV2>*>(data);
            return options_.registerDefinitions(tables.english, tables.local);
        }
        case Command::SetCoreOptionsDisplay:
            return options_.applyVisibility(*static_cast<const NativeCoreOptionVisibility*>(data));
        case Command::SetCoreOptionsUpdateDisplayCallback:
            options_.captureDisplayUpdate(*static_cast<const NativeCoreLegacyOptions::DisplayUpdate*>(data));
            return true;
        case Command::SetVariable:
            return options_.setNativeValue(*static_cast<const NativeCoreVariable*>(data));
        case Command::SetSupportNoGame: supportsNoGame_ = *static_cast<const unsigned char*>(data) != 0; return true;
        case Command::SetPerformanceLevel:
            performanceLevel_ = *static_cast<const unsigned*>(data);
            return true;
        case Command::SetKeyboardCallback: keyboard_ = *static_cast<const KeyboardCallback*>(data); return true;
        case Command::GetRumbleInterface:
            if (!interfaces_.rumble) return false;
            *static_cast<NativeCoreEnvironmentInterfaces::Rumble*>(data) = interfaces_.rumble;
            return true;
        case Command::GetLogInterface:
            if (!interfaces_.log) return false;
            *static_cast<NativeCoreEnvironmentInterfaces::Log*>(data) = interfaces_.log;
            return true;
        case Command::GetInputDeviceCapabilities:
            if (!interfaces_.inputCapabilities) return false;
            *static_cast<std::uint64_t*>(data) = *interfaces_.inputCapabilities;
            return true;
        case Command::GetLedInterface:
            if (!interfaces_.led) return false;
            *static_cast<NativeCoreEnvironmentInterfaces::Led*>(data) = interfaces_.led;
            return true;
        case Command::GetSensorInterface:
            if (!interfaces_.sensor.setState || !interfaces_.sensor.getInput) return false;
            *static_cast<NativeCoreSensorInterface*>(data) = interfaces_.sensor;
            return true;
        case Command::GetMessageInterfaceVersion:
            *static_cast<unsigned*>(data) = MessageInterfaceVersion;
            return true;
        case Command::SetMessage: {
            const auto& message = *static_cast<const NativeCoreMessage*>(data);
            const auto content = QString::fromUtf8(message.text);
            if (!content.trimmed().isEmpty()) {
                diagnostics_.append(content);
                messages_.append({content, message.frames});
            }
            return true;
        }
        case Command::SetMessageExtended: {
            const auto& message = *static_cast<const NativeCoreExtendedMessage*>(data);
            const auto content = QString::fromUtf8(message.text);
            if (!content.trimmed().isEmpty()) {
                diagnostics_.append(content);
                extendedMessages_.append({content, message.durationMilliseconds, message.priority,
                    message.level, message.target, message.type, message.progress});
            }
            return true;
        }
        case Command::SetPixelFormat:
            switch (static_cast<NativePixelFormat>(*static_cast<const unsigned*>(data))) {
            case NativePixelFormat::Rgb1555: video_.setPixelFormat(PixelFormat::Rgb1555); return true;
            case NativePixelFormat::Xrgb8888: video_.setPixelFormat(PixelFormat::Xrgb8888); return true;
            case NativePixelFormat::Rgb565: video_.setPixelFormat(PixelFormat::Rgb565); return true;
            }
            return false;
        case Command::SetSystemAvInfo: applyAvInfo(*static_cast<const NativeCoreAvInfo*>(data)); return true;
        case Command::SetGeometry:
            geometry_ = *static_cast<const NativeCoreGeometry*>(data);
            video_.applyGeometry(geometry_);
            return true;
        default: return false;
        }
    } catch (...) {
        if (!failure_) failure_ = std::current_exception();
        return false;
    }
}

void NativeCoreEnvironment::publishKeyboard(const QSet<EmulationKey>& keys, const QHash<EmulationKey, quint32>& codes) {
    if (!keyboard_) {
        previousKeys_.clear();
        return;
    }
    static const QSet modifiers{EmulationKey::LeftShift, EmulationKey::RightShift, EmulationKey::LeftControl,
        EmulationKey::RightControl, EmulationKey::LeftAlt, EmulationKey::RightAlt};
    for (const auto& event : InputProcessing::nativeKeyboardTransitions(keys, previousKeys_, codes, modifiers)) {
        if (!keyboard_) break;
        keyboard_(event.pressed, event.code, event.character, event.modifiers);
    }
    previousKeys_ = keys;
}

void NativeCoreEnvironment::publishAudioStatus(const std::optional<HostProtocol::AudioBufferStatus>& status) const {
    if (!audioBufferStatus_) return;
    if (!status || !status->active || status->capacityFrames <= 0 || status->sampleRate <= 0) {
        audioBufferStatus_(false, 0, false);
        return;
    }
    const auto percent = qint64(status->bufferedFrames) * FullBufferPercent / status->capacityFrames;
    const auto occupancy = unsigned(std::clamp(percent, qint64(0), qint64(FullBufferPercent)));
    const auto fps = std::isnan(framesPerSecond_) ? framesPerSecond_ : std::max(1.0, framesPerSecond_);
    audioBufferStatus_(true, occupancy, status->bufferedFrames < std::ceil(status->sampleRate / fps));
}

const QList<NativeCoreOption>& NativeCoreEnvironment::options() const noexcept { return options_.options(); }

const QList<NativeCoreOptionCategoryInfo>& NativeCoreEnvironment::optionCategories() const noexcept {
    return options_.categories();
}

const QList<NativeCoreInputDescription>& NativeCoreEnvironment::inputDescriptions() const noexcept {
    return inputDescriptions_;
}

const QList<QList<NativeCoreControllerDescription>>& NativeCoreEnvironment::controllerPorts() const noexcept {
    return controllerPorts_;
}

const QList<NativeCoreMemoryDescription>& NativeCoreEnvironment::memoryDescriptions() const noexcept {
    return memoryDescriptions_;
}

const QList<NativeCoreMessageInfo>& NativeCoreEnvironment::messages() const noexcept { return messages_; }
const QList<NativeCoreExtendedMessageInfo>& NativeCoreEnvironment::extendedMessages() const noexcept { return extendedMessages_; }
NativeCoreGeometry NativeCoreEnvironment::geometry() const noexcept { return geometry_; }
unsigned NativeCoreEnvironment::rotation() const noexcept { return rotation_; }
unsigned NativeCoreEnvironment::performanceLevel() const noexcept { return performanceLevel_; }

void NativeCoreEnvironment::clearBorrowedReferences() noexcept {
    defaultFiles_.reset();
    previousKeys_.clear();
    keyboard_ = nullptr;
    audioBufferStatus_ = nullptr;
    options_.captureDisplayUpdate(nullptr);
    if (interfaces_.disks) interfaces_.disks->clear();
    memoryDescriptions_.clear();
}

void NativeCoreEnvironment::clearOutputs() {
    shutdownRequested_ = false;
    audio_.clear();
    video_.clear();
}

void NativeCoreEnvironment::setOption(const QString& key, const QString& value) { options_.setValue(key, value); }

void NativeCoreEnvironment::recordDiagnostic(const char* utf8) noexcept {
    if (!utf8 || !*utf8) return;
    try { diagnostics_.append(QString::fromUtf8(utf8)); }
    catch (...) {
        if (!failure_) failure_ = std::current_exception();
    }
}

const QStringList& NativeCoreEnvironment::diagnostics() const noexcept { return diagnostics_; }
std::exception_ptr NativeCoreEnvironment::failure() const noexcept { return failure_; }
void NativeCoreEnvironment::rethrowCallbackFailure() const {
    for (const auto& error : {failure_, audio_.failure(), video_.failure()}) {
        if (error) std::rethrow_exception(error);
    }
}
bool NativeCoreEnvironment::supportsNoGame() const noexcept { return supportsNoGame_; }
bool NativeCoreEnvironment::supportsAchievements() const noexcept { return supportsAchievements_; }
bool NativeCoreEnvironment::shutdownRequested() const noexcept { return shutdownRequested_; }
double NativeCoreEnvironment::framesPerSecond() const noexcept { return framesPerSecond_; }
int NativeCoreEnvironment::minimumAudioLatencyMilliseconds() const noexcept { return minimumAudioLatency_; }
}
