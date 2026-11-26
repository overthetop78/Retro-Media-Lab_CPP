#include <RetroMediaLab/Emulation/NativeCoreLibrary.h>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <filesystem>
#include <utility>

namespace RetroMediaLab::Emulation {
namespace {
constexpr auto ErrorKey = "Emulation.Error.EmulatorRejected";
constexpr auto ApiVersionExport = "retro_api_version";
constexpr auto SystemInfoExport = "retro_get_system_info";
constexpr auto SetEnvironmentExport = "retro_set_environment";
constexpr auto SetVideoRefreshExport = "retro_set_video_refresh";
constexpr auto SetAudioSampleExport = "retro_set_audio_sample";
constexpr auto SetAudioSampleBatchExport = "retro_set_audio_sample_batch";
constexpr auto SetInputPollExport = "retro_set_input_poll";
constexpr auto SetInputStateExport = "retro_set_input_state";
constexpr auto InitializeExport = "retro_init";
constexpr auto DeinitializeExport = "retro_deinit";
constexpr auto GetSystemAvInfoExport = "retro_get_system_av_info";
constexpr auto SetControllerPortDeviceExport = "retro_set_controller_port_device";
constexpr auto ResetExport = "retro_reset";
constexpr auto RunExport = "retro_run";
constexpr auto LoadGameExport = "retro_load_game";
constexpr auto UnloadGameExport = "retro_unload_game";
constexpr auto GetRegionExport = "retro_get_region";
constexpr auto GetMemoryDataExport = "retro_get_memory_data";
constexpr auto GetMemorySizeExport = "retro_get_memory_size";
constexpr auto GetSerializedSizeExport = "retro_serialize_size";
constexpr auto SerializeExport = "retro_serialize";
constexpr auto UnserializeExport = "retro_unserialize";
constexpr auto LoadGameSpecialExport = "retro_load_game_special";
constexpr auto CoreDirectoryTemplate = "Core-XXXXXX";
constexpr QChar ExtensionSeparator = u'|';
constexpr QChar ExtensionDot = u'.';

template<class Function>
Function bind(NativeCoreLibrary& library, const char* name) {
    return reinterpret_cast<Function>(library.resolve(name));
}

QString copyText(const char* text) {
    return text ? QString::fromUtf8(text) : QString{};
}
}

NativeCoreLibraryError::NativeCoreLibraryError(NativeCoreLibraryFailure failure, QString detail)
    : std::runtime_error(ErrorKey), failure_(failure), detail_(std::move(detail)) {}

NativeCoreLibraryFailure NativeCoreLibraryError::failure() const noexcept { return failure_; }
const QString& NativeCoreLibraryError::detail() const noexcept { return detail_; }

void NativeCoreLibrary::validatePath(const QString& path) {
    if (!std::filesystem::path(path.toStdU16String()).is_absolute())
        throw NativeCoreLibraryError(NativeCoreLibraryFailure::RelativePath, path);
    if (!QFileInfo(path).isFile())
        throw NativeCoreLibraryError(NativeCoreLibraryFailure::MissingFile, path);
}

NativeCoreLibrary::NativeCoreLibrary(const QString& absolutePath)
    : NativeCoreLibrary(absolutePath, {}) {}

NativeCoreLibrary::NativeCoreLibrary(const QString& absolutePath, const QString& isolationDirectory) {
    validatePath(absolutePath);
    auto loadPath = absolutePath;
    if (!isolationDirectory.isEmpty()) {
        const QDir parent(QFileInfo(isolationDirectory).absoluteFilePath());
        if (!QDir().mkpath(parent.path()))
            throw NativeCoreLibraryError(NativeCoreLibraryFailure::StagingFailed, parent.path());
        staging_ = std::make_unique<QTemporaryDir>(parent.filePath(QLatin1String(CoreDirectoryTemplate)));
        if (!staging_->isValid())
            throw NativeCoreLibraryError(NativeCoreLibraryFailure::StagingFailed, parent.path());
        loadPath = staging_->filePath(QFileInfo(absolutePath).fileName());
        if (!QFile::copy(absolutePath, loadPath))
            throw NativeCoreLibraryError(NativeCoreLibraryFailure::StagingFailed, loadPath);
    }
    library_.setFileName(loadPath);
    if (!library_.load())
        throw NativeCoreLibraryError(NativeCoreLibraryFailure::LoadFailed, loadPath);
}

NativeCoreLibrary::~NativeCoreLibrary() { library_.unload(); }

QFunctionPointer NativeCoreLibrary::resolve(const char* exportName) {
    const auto entry = library_.resolve(exportName);
    if (!entry) throw NativeCoreLibraryError(NativeCoreLibraryFailure::MissingExport, copyText(exportName));
    return entry;
}

NativeCoreExports NativeCoreLibrary::resolveExports(bool requireLoadGameSpecial) {
    using Exports = NativeCoreExports;
    return {
        bind<decltype(Exports::setEnvironment)>(*this, SetEnvironmentExport),
        bind<decltype(Exports::setVideoRefresh)>(*this, SetVideoRefreshExport),
        bind<decltype(Exports::setAudioSample)>(*this, SetAudioSampleExport),
        bind<decltype(Exports::setAudioSampleBatch)>(*this, SetAudioSampleBatchExport),
        bind<decltype(Exports::setInputPoll)>(*this, SetInputPollExport),
        bind<decltype(Exports::setInputState)>(*this, SetInputStateExport),
        bind<decltype(Exports::initialize)>(*this, InitializeExport),
        bind<decltype(Exports::deinitialize)>(*this, DeinitializeExport),
        bind<Exports::SystemInfo>(*this, SystemInfoExport),
        bind<decltype(Exports::getSystemAvInfo)>(*this, GetSystemAvInfoExport),
        bind<decltype(Exports::setControllerPortDevice)>(*this, SetControllerPortDeviceExport),
        bind<decltype(Exports::reset)>(*this, ResetExport),
        bind<decltype(Exports::run)>(*this, RunExport),
        bind<decltype(Exports::loadGame)>(*this, LoadGameExport),
        bind<decltype(Exports::unloadGame)>(*this, UnloadGameExport),
        bind<decltype(Exports::getRegion)>(*this, GetRegionExport),
        bind<decltype(Exports::getMemoryData)>(*this, GetMemoryDataExport),
        bind<decltype(Exports::getMemorySize)>(*this, GetMemorySizeExport),
        bind<decltype(Exports::getSerializedSize)>(*this, GetSerializedSizeExport),
        bind<decltype(Exports::serialize)>(*this, SerializeExport),
        bind<decltype(Exports::unserialize)>(*this, UnserializeExport),
        requireLoadGameSpecial ? resolveSpecialContent() : nullptr,
    };
}

NativeCoreExports::LoadGameSpecial NativeCoreLibrary::resolveSpecialContent() {
    return bind<NativeCoreExports::LoadGameSpecial>(*this, LoadGameSpecialExport);
}

QStringList parseNativeCoreExtensions(const char* extensions) {
    QStringList result;
    for (const auto& part : copyText(extensions).split(ExtensionSeparator, Qt::SkipEmptyParts)) {
        auto extension = part.trimmed();
        if (extension.isEmpty()) continue;
        qsizetype first = 0;
        while (first < extension.size() && extension.at(first) == ExtensionDot) ++first;
        extension.remove(0, first);
        if (!result.contains(extension, Qt::CaseInsensitive)) result.append(std::move(extension));
    }
    return result;
}

NativeCoreMetadata NativeCoreLibrary::inspect(const NativeCoreExports& exports,
    unsigned expectedApiVersion, const QString& expectedName) {
    const auto apiVersion = bind<unsigned (*)()>(*this, ApiVersionExport);
    const auto version = apiVersion();
    if (version != expectedApiVersion)
        throw NativeCoreLibraryError(NativeCoreLibraryFailure::ApiVersion, QString::number(version));
    NativeCoreSystemInfo info{};
    exports.getSystemInfo(&info);
    auto name = copyText(info.libraryName);
    if (name.compare(expectedName, Qt::CaseInsensitive) != 0)
        throw NativeCoreLibraryError(NativeCoreLibraryFailure::Identity, name);
    return {std::move(name), copyText(info.libraryVersion), parseNativeCoreExtensions(info.validExtensions),
        info.needsFullPath, info.blockExtract};
}

NativeCoreMetadata inspectNativeCoreLibrary(const QString& path, unsigned expectedApiVersion,
    const QString& expectedName, bool requireLoadGameSpecial) {
    NativeCoreLibrary library(path);
    return library.inspect(library.resolveExports(requireLoadGameSpecial), expectedApiVersion, expectedName);
}
}
