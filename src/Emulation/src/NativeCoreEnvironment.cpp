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
constexpr unsigned MaximumInputDescriptors = 4096;
constexpr unsigned MaximumControllerPorts = 32;
constexpr unsigned MaximumControllerTypes = 256;
constexpr unsigned MaximumMemoryDescriptors = 4096;
constexpr unsigned LastRotation = 3;
constexpr unsigned MessageInterfaceVersion = 1;

}

QList<NativeCoreInputDescription> copyNativeCoreInputDescriptors(const NativeCoreInputDescriptor* data, unsigned maximumCount) {
    QList<NativeCoreInputDescription> inputs;
    if (!data) return inputs;
    for (unsigned index = 0; index < maximumCount && data[index].description; ++index) {
        const auto& item = data[index];
        inputs.append({item.port, item.device, item.index, item.id, QString::fromUtf8(item.description)});
    }
    return inputs;
}

QList<QList<NativeCoreControllerDescription>> copyNativeCoreControllerPorts(
    const NativeCoreControllerInfo* data, unsigned maximumPorts, unsigned maximumDevices) {
    QList<QList<NativeCoreControllerDescription>> ports;
    tryCopyNativeCoreControllerPorts(data, maximumPorts, maximumDevices, false, false, ports);
    return ports;
}

bool tryCopyNativeCoreControllerPorts(const NativeCoreControllerInfo* data, unsigned maximumPorts,
    unsigned maximumDevices, bool rejectOverflow, bool skipEmptyNames,
    QList<QList<NativeCoreControllerDescription>>& result) {
    result.clear();
    if (!data) return true;
    for (unsigned index = 0; index < maximumPorts && data[index].types && data[index].count; ++index) {
        const auto& port = data[index];
        if (rejectOverflow && port.count > maximumDevices) return false;
        const auto count = std::min(port.count, maximumDevices);
        QList<NativeCoreControllerDescription> types;
        for (unsigned device = 0; device < count; ++device) {
            const auto& type = port.types[device];
            auto name = type.description ? QString::fromUtf8(type.description) : QStringLiteral("");
            if (!skipEmptyNames || !name.trimmed().isEmpty())
                types.append({std::move(name), type.id});
        }
        result.append(std::move(types));
    }
    return true;
}

QList<NativeCoreMemoryDescription> copyNativeCoreMemoryMap(const NativeCoreMemoryMap* map, unsigned maximumCount) {
    QList<NativeCoreMemoryDescription> memory;
    if (!map || !map->descriptors) return memory;
    const auto count = std::min(map->count, maximumCount);
    memory.reserve(count);
    for (unsigned index = 0; index < count; ++index) {
        const auto& item = map->descriptors[index];
        memory.append({item.flags, item.pointer, item.offset, item.start, item.select, item.disconnect,
            item.length, QString::fromUtf8(item.addressSpace)});
    }
    return memory;
}

namespace {
enum class NativeLanguage : unsigned {
    English = 0, Japanese = 1, French = 2, Spanish = 3, German = 4, Italian = 5, Dutch = 6,
    PortugueseBrazil = 7, PortuguesePortugal = 8, Russian = 9, Korean = 10,
    ChineseTraditional = 11, ChineseSimplified = 12, Polish = 14, Vietnamese = 15,
    Arabic = 16, Greek = 17, Turkish = 18, Hebrew = 21, Finnish = 23, Indonesian = 24,
    Swedish = 25, Ukrainian = 26, Czech = 27, Hungarian = 31, Norwegian = 34, Thai = 36
};

NativeLanguage nativeLanguage(const QLocale& locale) noexcept {
    switch (locale.language()) {
    case QLocale::Japanese: return NativeLanguage::Japanese;
    case QLocale::French: return NativeLanguage::French;
    case QLocale::Spanish: return NativeLanguage::Spanish;
    case QLocale::German: return NativeLanguage::German;
    case QLocale::Italian: return NativeLanguage::Italian;
    case QLocale::Dutch: return NativeLanguage::Dutch;
    case QLocale::Portuguese:
        return locale.territory() == QLocale::Brazil ? NativeLanguage::PortugueseBrazil : NativeLanguage::PortuguesePortugal;
    case QLocale::Russian: return NativeLanguage::Russian;
    case QLocale::Korean: return NativeLanguage::Korean;
    case QLocale::Chinese:
        return locale.script() == QLocale::TraditionalHanScript ? NativeLanguage::ChineseTraditional : NativeLanguage::ChineseSimplified;
    case QLocale::Polish: return NativeLanguage::Polish;
    case QLocale::Vietnamese: return NativeLanguage::Vietnamese;
    case QLocale::Arabic: return NativeLanguage::Arabic;
    case QLocale::Greek: return NativeLanguage::Greek;
    case QLocale::Turkish: return NativeLanguage::Turkish;
    case QLocale::Hebrew: return NativeLanguage::Hebrew;
    case QLocale::Finnish: return NativeLanguage::Finnish;
    case QLocale::Indonesian: return NativeLanguage::Indonesian;
    case QLocale::Swedish: return NativeLanguage::Swedish;
    case QLocale::Ukrainian: return NativeLanguage::Ukrainian;
    case QLocale::Czech: return NativeLanguage::Czech;
    case QLocale::Hungarian: return NativeLanguage::Hungarian;
    case QLocale::NorwegianBokmal: return NativeLanguage::Norwegian;
    case QLocale::Thai: return NativeLanguage::Thai;
    default: return NativeLanguage::English;
    }
}

enum class NativePixelFormat : unsigned { Rgb1555 = 0, Xrgb8888 = 1, Rgb565 = 2 };

QString ensureDirectory(const QString& path) {
    if (path.isEmpty() || !QDir().mkpath(path)) throw std::runtime_error(ErrorKey);
    return QDir::toNativeSeparators(QDir::cleanPath(QFileInfo(path).absoluteFilePath()));
}

int roundedSampleRate(double value) {
    const double lower = std::floor(value);
    constexpr double midpoint = 0.5;
    const double rounded = value - lower == midpoint
        ? (std::fmod(lower, 2.0) == 0.0 ? lower : lower + 1.0)
        : std::round(value);
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

QString copyNativeLogTemplate(const char* format, QStringView argumentMarker, QStringView conversionCharacters) {
    if (!format) return {};
    const auto source = QString::fromUtf8(format).trimmed();
    QString result;
    result.reserve(source.size());
    for (qsizetype index = 0; index < source.size(); ++index) {
        const auto character = source.at(index);
        if (character != u'%' || (index + 1 < source.size() && source.at(index + 1) == u'%')) {
            result.append(character);
            if (character == u'%') ++index;
            continue;
        }
        result.append(argumentMarker);
        while (index + 1 < source.size()) {
            if (conversionCharacters.contains(source.at(++index))) break;
        }
    }
    return result;
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
    geometry_ = info.geometry;
    video_.applyGeometry(geometry_);

    const auto frames = info.timing.framesPerSecond;
    if (std::isfinite(frames) && frames > 0) framesPerSecond_ = frames;

    const auto rate = info.timing.sampleRate;
    if (std::isfinite(rate) && rate > 0 && rate <= std::numeric_limits<int>::max())
        audio_.setSampleRate(roundedSampleRate(rate));
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
            inputDescriptions_ = copyNativeCoreInputDescriptors(static_cast<const NativeCoreInputDescriptor*>(data), MaximumInputDescriptors);
            return data != nullptr;
        case Command::SetControllerInfo:
            controllerPorts_ = copyNativeCoreControllerPorts(static_cast<const NativeCoreControllerInfo*>(data),
                MaximumControllerPorts, MaximumControllerTypes);
            return data != nullptr;
        case Command::SetMemoryMaps:
            memoryDescriptions_ = copyNativeCoreMemoryMap(static_cast<const NativeCoreMemoryMap*>(data), MaximumMemoryDescriptors);
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
            *static_cast<unsigned*>(data) = static_cast<unsigned>(nativeLanguage(uiLocale_));
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
            if (!content.isEmpty()) {
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
    minimumAudioLatency_ = 0;
    options_.releaseNativeReferences();
    if (interfaces_.disks) interfaces_.disks->clear();
    memoryDescriptions_.clear();
    diagnostics_.clear();
    messages_.clear();
    extendedMessages_.clear();
    failure_ = nullptr;
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
