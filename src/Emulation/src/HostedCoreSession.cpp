#include <RetroMediaLab/Emulation/HostedCoreSession.h>
#include <RetroMediaLab/Emulation/StorageSettings.h>
#include <QBuffer>
#include <QDataStream>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QSet>
#include <limits>

namespace RetroMediaLab::Emulation {

QHash<MediaKind, bool> HostedCoreStatus::firstSlotActivity(bool includeOpticalDisc) const {
    return StorageRules::firstSlotActivityFromLeds(leds, includeOpticalDisc);
}

namespace {
constexpr auto ExtensionSeparator = u'|';
constexpr auto MediaPathKey = "path";
constexpr auto MediaSlotKey = "slot";
constexpr auto MediaSlotIndexKey = "index";
constexpr auto MediaLegacySlotIndexKey = "slotIndex";
constexpr auto MediaMountOrderKey = "mountOrder";
constexpr qint32 FirstDiskIndex = 0;
constexpr auto DiskImageCountKey = "imageCount";
constexpr auto DiskCurrentIndexKey = "currentIndex";
constexpr auto DiskEjectedKey = "isEjected";
constexpr auto DiskImagesKey = "images";
constexpr auto DiskImageIndexKey = "index";
constexpr auto DiskImageLabelKey = "label";

QStringList contentExtensions(const QString& extensions) {
    auto result = extensions.split(ExtensionSeparator, Qt::SkipEmptyParts);
    QSet<QString> seen;
    result.removeIf([&](const QString& extension) {
        const auto key = extension.toCaseFolded();
        if (seen.contains(key)) return true;
        seen.insert(key);
        return false;
    });
    return result;
}

std::optional<qint32> jsonInteger(const QJsonValue& value) {
    if (!value.isDouble()) return {};
    const auto number = value.toInteger(std::numeric_limits<qint64>::min());
    if (number < std::numeric_limits<qint32>::min() || number > std::numeric_limits<qint32>::max()) return {};
    return static_cast<qint32>(number);
}

std::optional<qint32> insertedDiskIndex(const QJsonObject& media, HostedMediaDiskIndex source) {
    if (source == HostedMediaDiskIndex::Slot) {
        const auto slot = media.value(QLatin1String(MediaSlotKey)).toObject();
        return jsonInteger(slot.value(QLatin1String(MediaSlotIndexKey)));
    }
    if (source == HostedMediaDiskIndex::SlotIndexOrMountOrder || source == HostedMediaDiskIndex::SlotIndexOrFirstDisk) {
        const auto index = media.value(QLatin1String(MediaLegacySlotIndexKey));
        if (!index.isUndefined() && !index.isNull()) return jsonInteger(index);
        if (source == HostedMediaDiskIndex::SlotIndexOrFirstDisk) return FirstDiskIndex;
    }
    return jsonInteger(media.value(QLatin1String(MediaMountOrderKey)));
}

std::optional<QString> jsonNullableText(const QJsonValue& value) {
    if (value.isUndefined() || value.isNull()) return QString{};
    if (!value.isString()) return {};
    return value.toString();
}

std::optional<HostedCoreDiskReport> readDiskReport(QIODevice& response) {
    const auto raw = HostProtocol::readString(response);
    if (!raw || !response.atEnd()) return {};
    const auto json = QJsonDocument::fromJson(raw->toUtf8());
    if (!json.isObject()) return {};
    const auto object = json.object();
    const auto count = jsonInteger(object.value(QLatin1String(DiskImageCountKey)));
    const auto current = jsonInteger(object.value(QLatin1String(DiskCurrentIndexKey)));
    const auto ejected = object.value(QLatin1String(DiskEjectedKey));
    const auto images = object.value(QLatin1String(DiskImagesKey));
    if (!count || !current || !ejected.isBool() || !images.isArray()) return {};
    HostedCoreDiskReport report{*count, *current, ejected.toBool(), {}};
    for (const auto& entry : images.toArray()) {
        if (!entry.isObject()) return {};
        const auto image = entry.toObject();
        const auto index = jsonInteger(image.value(QLatin1String(DiskImageIndexKey)));
        const auto path = jsonNullableText(image.value(QLatin1String(MediaPathKey)));
        const auto label = jsonNullableText(image.value(QLatin1String(DiskImageLabelKey)));
        if (!index || !path || !label) return {};
        report.images.append({*index, *path, *label});
    }
    return report;
}

std::optional<QJsonArray> readJsonArray(QIODevice& input) {
    const auto text = HostProtocol::readString(input);
    if (!text) return {};
    if (*text == QLatin1String("null")) return QJsonArray{};
    const auto document = QJsonDocument::fromJson(text->toUtf8());
    if (!document.isArray()) return {};
    return document.array();
}

std::optional<QStringList> readDiagnostics(QIODevice& input) {
    const auto list = readJsonArray(input);
    if (!list) return {};
    QStringList result;
    result.reserve(list->size());
    for (const auto& item : *list) {
        if (!item.isString()) return {};
        result.append(item.toString());
    }
    return result;
}

bool readTiming(QIODevice& input, HostedCoreStatus& status) {
    QDataStream stream(&input);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::DoublePrecision);
    stream >> status.framesPerSecond >> status.sampleRate >> status.minimumAudioLatencyMs;
    return stream.status() == QDataStream::Ok;
}

bool readStatusTail(QIODevice& input, HostedCoreStatus& status, bool shutdownAndFeedback) {
    auto leds = HostProtocol::readLedStates(input);
    if (!leds) return false;
    status.leds = std::move(*leds);
    if (shutdownAndFeedback) {
        const auto shutdown = HostProtocol::readMediaChanges(input);
        auto feedback = HostProtocol::readControllerFeedback(input);
        if (!shutdown || !feedback) return false;
        status.shutdownRequested = *shutdown;
        status.controllerFeedback = std::move(*feedback);
    }
    return input.atEnd();
}
}

struct HostedCoreSession::VideoOutput {
    std::shared_ptr<const VideoFrame> latest;
    QList<std::shared_ptr<const VideoFrame>> previous;

    void receive(VideoFrame frame) {
        auto next = std::make_shared<VideoFrame>(std::move(frame));
        if (latest && latest->scanMode != ScanMode::Progressive && next->scanMode != ScanMode::Progressive &&
            next->scanMode != latest->scanMode && next->sequence > latest->sequence &&
            next->sequence - latest->sequence == 1 && next->timestamp > latest->timestamp) {
            if (previous.size() == IMachineVideoFieldHistory::MaximumFields) previous.removeFirst();
            previous.append(latest);
        } else {
            previous.clear();
        }
        latest = std::move(next);
    }
};

HostedCoreSession::HostedCoreSession(HostedCoreSessionOptions options) : options_(std::move(options)) {}
HostedCoreSession::~HostedCoreSession() { close(); }

HostedCoreOperation HostedCoreSession::request(HostedCoreCommand command, QByteArrayView payload,
    QByteArray& response, int timeoutMs, std::stop_token token) {
    if (timeoutMs <= 0) return {HostedCoreResult::InvalidRequest, {}};
    auto reply = transport_->command(options_.version, command, payload, timeoutMs, token);
    if (!reply) {
        hostFaulted_ = true;
        transport_.reset();
        close();
        return {HostedCoreResult::CommunicationFailure, {}};
    }
    if (reply->error) return {HostedCoreResult::RemoteError, std::move(*reply->error)};
    response = std::move(reply->payload);
    return {HostedCoreResult::Success, {}};
}

HostedCoreOperation HostedCoreSession::initialize(QByteArrayView payload, int timeoutMs, std::stop_token token) {
    if (transport_) return {HostedCoreResult::InvalidState, {}};
    if (timeoutMs <= 0) return {HostedCoreResult::InvalidRequest, {}};
    if (token.stop_requested()) return {HostedCoreResult::CommunicationFailure, {}};
    qint64 startedProcessId = 0;
    transport_ = HostedCoreTransport::start(options_.transport, startedProcessId, token);
    if (startedProcessId > 0) hostProcessId_ = startedProcessId;
    hostFaulted_ = !transport_;
    if (!transport_) return {HostedCoreResult::CommunicationFailure, {}};
    QByteArray response;
    auto operation = request(HostedCoreCommand::Initialize, payload, response, timeoutMs, token);
    if (!operation) {
        close();
        return operation;
    }
    QBuffer input(&response);
    input.open(QIODevice::ReadOnly);
    HostedCoreStatus nextStatus;
    const auto sha = HostProtocol::readString(input);
    if (sha && readTiming(input, nextStatus)) {
        const auto supportsStates = HostProtocol::readMediaChanges(input);
        const auto runtime = HostProtocol::readRuntimeStatus(input);
        auto options = readJsonArray(input);
        auto diagnostics = readDiagnostics(input);
        auto name = HostProtocol::readString(input);
        auto version = HostProtocol::readString(input);
        const auto extensions = HostProtocol::readString(input);
        if (options_.initializationDiskSelection)
            nextStatus.diskSelection = HostProtocol::readDiskSelection(input);
        if (supportsStates && runtime && options && diagnostics && name && version && extensions &&
            (!options_.initializationDiskSelection || nextStatus.diskSelection) &&
            readStatusTail(input, nextStatus, options_.shutdownAndFeedback)) {
            nextStatus.supportsSaveStates = *supportsStates;
            nextStatus.runtime = *runtime;
            status_ = std::move(nextStatus);
            info_ = {*sha, std::move(*name), std::move(*version), contentExtensions(*extensions),
                std::move(*options)};
            diagnostics_ = std::move(*diagnostics);
            initialized_ = true;
            return operation;
        }
    }
    return invalidReply();
}

HostedCoreOperation HostedCoreSession::restart(QByteArrayView payload, int timeoutMs, std::stop_token token) {
    if (!initialized_) return {HostedCoreResult::InvalidState, {}};
    if (timeoutMs <= 0) return {HostedCoreResult::InvalidRequest, {}};
    if (token.stop_requested()) return {HostedCoreResult::CommunicationFailure, {}};
    close();
    return initialize(payload, timeoutMs, token);
}

HostedCoreOperation HostedCoreSession::hardReset(int timeoutMs, std::stop_token token) {
    const auto result = executeEmptyReply(HostedCoreCommand::HardReset, {}, timeoutMs, token);
    if (result) clearOutputs(false);
    return result;
}

HostedCoreOperation HostedCoreSession::softReset(int timeoutMs, std::stop_token token) {
    const auto result = executeEmptyReply(HostedCoreCommand::SoftReset, {}, timeoutMs, token);
    if (result) clearOutputs(false);
    return result;
}

HostedCoreOperation HostedCoreSession::stop(int timeoutMs, std::stop_token token) {
    if (timeoutMs <= 0) return {HostedCoreResult::InvalidRequest, {}};
    const auto result = executeEmptyReply(HostedCoreCommand::Stop, {}, timeoutMs, token);
    if (result) clearOutputs(true);
    if (options_.stopClosesHost) close();
    return result;
}

HostedCoreOperation HostedCoreSession::runFrame(const HostProtocol::CoreInput& keys,
    const std::optional<HostProtocol::AudioBufferStatus>& audioStatus, HostedCoreFrame& output, int timeoutMs,
    std::stop_token token) {
    if (!initialized_) return {HostedCoreResult::InvalidState, {}};
    QBuffer payload;
    payload.open(QIODevice::ReadWrite);
    if (!HostProtocol::writeInput(payload, keys) || !HostProtocol::writeAudioBufferStatus(payload, audioStatus))
        return {HostedCoreResult::InvalidRequest, {}};

    QByteArray response;
    auto operation = request(HostedCoreCommand::RunFrame, payload.data(), response, timeoutMs, token);
    if (!operation) return operation;
    QBuffer input(&response);
    input.open(QIODevice::ReadOnly);
    auto nextStatus = status_;
    std::optional<VideoFrame> frame;
    if (transport_->videoMemory().readFrame(input, frame)) {
        auto audio = HostProtocol::readAudio(input);
        if (audio && readTiming(input, nextStatus)) {
            const auto runtime = HostProtocol::readRuntimeStatus(input);
            const auto supportsStates = options_.frameStateSupport
                ? HostProtocol::readMediaChanges(input) : std::optional{status_.supportsSaveStates};
            const auto disk = options_.frameDiskSelection ? HostProtocol::readDiskSelection(input) : std::nullopt;
            const auto diagnosticsChanged = HostProtocol::readMediaChanges(input);
            auto diagnostics = diagnosticsChanged.value_or(false) ? readDiagnostics(input) : std::nullopt;
            if (runtime && supportsStates && (!options_.frameDiskSelection || disk) && diagnosticsChanged &&
                (!*diagnosticsChanged || diagnostics) && readStatusTail(input, nextStatus, options_.shutdownAndFeedback)) {
                nextStatus.runtime = *runtime;
                nextStatus.supportsSaveStates = *supportsStates;
                if (disk) nextStatus.diskSelection = disk;
                if (frame) {
                    if (!videoOutput_) videoOutput_ = std::make_shared<VideoOutput>();
                    videoOutput_->receive(std::move(*frame));
                }
                if (diagnostics) diagnostics_ = std::move(*diagnostics);
                status_ = std::move(nextStatus);
                output = {std::move(*audio), disk};
                return operation;
            }
        }
    }
    return invalidReply();
}

HostedCoreOperation HostedCoreSession::execute(HostedCoreCommand command, QByteArrayView payload, QByteArray& response,
    int timeoutMs, std::stop_token token) {
    if (!initialized_) return {HostedCoreResult::InvalidState, {}};
    if (command == HostedCoreCommand::Initialize || command == HostedCoreCommand::RunFrame ||
        command == HostedCoreCommand::Dispose) return {HostedCoreResult::InvalidRequest, {}};
    return request(command, payload, response, timeoutMs, token);
}

HostedCoreOperation HostedCoreSession::invalidReply() {
    hostFaulted_ = true;
    transport_.reset();
    close();
    return {HostedCoreResult::InvalidReply, {}};
}

HostedCoreOperation HostedCoreSession::executeEmptyReply(HostedCoreCommand command,
    const std::function<bool(QIODevice&)>& write, int timeoutMs, std::stop_token token) {
    if (!initialized_) return {HostedCoreResult::InvalidState, {}};
    QBuffer payload;
    payload.open(QIODevice::WriteOnly);
    if (write && !write(payload)) return {HostedCoreResult::InvalidRequest, {}};
    QByteArray response;
    const auto result = execute(command, payload.data(), response, timeoutMs, token);
    return result && !response.isEmpty() ? invalidReply() : result;
}

HostedCoreOperation HostedCoreSession::saveState(QByteArray& state, int timeoutMs, std::stop_token token) {
    QByteArray response;
    const auto result = execute(HostedCoreCommand::SaveState, {}, response, timeoutMs, token);
    if (!result) return result;
    QBuffer input(&response);
    input.open(QIODevice::ReadOnly);
    auto value = HostProtocol::readBytes(input);
    if (!value || !input.atEnd()) return invalidReply();
    state = std::move(*value);
    return result;
}

HostedCoreOperation HostedCoreSession::loadState(QByteArrayView state, int timeoutMs, std::stop_token token) {
    const auto result = executeEmptyReply(HostedCoreCommand::LoadState,
        [=](QIODevice& payload) { return HostProtocol::writeBytes(payload, state); }, timeoutMs, token);
    if (result) clearOutputs(false);
    return result;
}

HostedCoreOperation HostedCoreSession::insertMedia(const QJsonObject& media, int timeoutMs, std::stop_token token) {
    const auto result = executeEmptyReply(HostedCoreCommand::InsertMedia, [&](QIODevice& payload) {
        auto document = media;
        const auto path = document.value(QLatin1String(MediaPathKey)).toString();
        if (path.isEmpty()) return false;
        document.insert(QLatin1String(MediaPathKey), QDir::toNativeSeparators(QFileInfo(path).absoluteFilePath()));
        return HostProtocol::writeString(payload, QString::fromUtf8(QJsonDocument(document).toJson(QJsonDocument::Compact)));
    }, timeoutMs, token);
    if (result && status_.diskSelection) {
        const auto index = insertedDiskIndex(media, options_.insertDiskIndex);
        if (index && *index >= 0 && *index < status_.diskSelection->diskCount)
            status_.diskSelection->currentDiskIndex = *index;
    }
    return result;
}

HostedCoreOperation HostedCoreSession::ejectMedia(HostedMediaSlot slot, int timeoutMs, std::stop_token token) {
    const auto result = executeEmptyReply(HostedCoreCommand::EjectMedia,
        [=](QIODevice& payload) { return HostProtocol::writeMediaSlot(payload, slot); }, timeoutMs, token);
    if (result && status_.diskSelection) status_.diskSelection->currentDiskIndex = slot.index;
    return result;
}

HostedCoreOperation HostedCoreSession::saveMediaChanges(HostedMediaSlot slot, int timeoutMs, std::stop_token token) {
    return executeEmptyReply(HostedCoreCommand::SaveMediaChanges,
        [=](QIODevice& payload) { return HostProtocol::writeMediaSlot(payload, slot); }, timeoutMs, token);
}

HostedCoreOperation HostedCoreSession::hasUnsavedMediaChanges(HostedMediaSlot slot, bool& changed,
    int timeoutMs, std::stop_token token) {
    if (!initialized_) return {HostedCoreResult::InvalidState, {}};
    QBuffer payload;
    payload.open(QIODevice::WriteOnly);
    if (!HostProtocol::writeMediaSlot(payload, slot)) return {HostedCoreResult::InvalidRequest, {}};
    QByteArray response;
    const auto result = execute(HostedCoreCommand::HasUnsavedMediaChanges, payload.data(), response, timeoutMs, token);
    if (!result) return result;
    QBuffer input(&response);
    input.open(QIODevice::ReadOnly);
    const auto value = HostProtocol::readMediaChanges(input);
    if (!value || !input.atEnd()) return invalidReply();
    changed = *value;
    return result;
}

HostedCoreOperation HostedCoreSession::setOption(const QString& key, const QString& value, int timeoutMs, std::stop_token token) {
    return executeEmptyReply(HostedCoreCommand::SetOption,
        [&](QIODevice& payload) { return HostProtocol::writeOption(payload, key, value); }, timeoutMs, token);
}

HostedCoreOperation HostedCoreSession::setControllerPortDevice(qint32 port, qint32 device, int timeoutMs,
    std::stop_token token) {
    const auto result = executeEmptyReply(HostedCoreCommand::SetControllerPortDevice,
        [=](QIODevice& payload) { return HostProtocol::writeControllerPortDevice(payload, port, device); }, timeoutMs, token);
    if (result) {
        status_.controllerFeedback.removeIf([port](const auto& state) { return state.port == port; });
    }
    return result;
}

HostedCoreOperation HostedCoreSession::selectDisk(qint32 index, int timeoutMs, std::stop_token token) {
    const auto result = executeEmptyReply(HostedCoreCommand::SelectDisk,
        [=](QIODevice& payload) { return HostProtocol::writeIndex(payload, index); }, timeoutMs, token);
    if (result && status_.diskSelection) status_.diskSelection->currentDiskIndex = index;
    return result;
}

HostedCoreOperation HostedCoreSession::getDiskStatus(HostedCoreDiskReport& report, int timeoutMs, std::stop_token token) {
    QByteArray response;
    const auto result = execute(HostedCoreCommand::GetDiskStatus, {}, response, timeoutMs, token);
    if (!result) return result;
    QBuffer input(&response);
    input.open(QIODevice::ReadOnly);
    auto status = readDiskReport(input);
    if (!status) return invalidReply();
    report = std::move(*status);
    return result;
}

void HostedCoreSession::clearOutputs(bool shutdownRequested) noexcept {
    videoOutput_.reset();
    status_.runtime.bufferedAudioFrames = 0;
    status_.controllerFeedback.clear();
    status_.shutdownRequested = shutdownRequested;
}

void HostedCoreSession::close() noexcept {
    try {
        if (transport_ && transport_->connected()) {
            transport_->command(options_.version, HostedCoreCommand::Dispose, {},
                options_.transport.gracefulShutdownTimeoutMs);
        }
    } catch (...) {
        // The transport still releases the connection and process.
    }
    transport_.reset();
    initialized_ = false;
    info_ = {};
    status_ = {};
    diagnostics_.clear();
    videoOutput_.reset();
}

bool HostedCoreSession::initialized() const noexcept { return initialized_; }
HostProcessState HostedCoreSession::hostProcessState() const noexcept {
    if (hostFaulted_) return HostProcessState::Faulted;
    if (transport_ && transport_->isProcessRunning()) return HostProcessState::Running;
    return hostProcessId_ ? HostProcessState::Exited : HostProcessState::NotStarted;
}
std::optional<qint64> HostedCoreSession::hostProcessId() const noexcept { return hostProcessId_; }
const HostedCoreInfo& HostedCoreSession::info() const noexcept { return info_; }
const HostedCoreStatus& HostedCoreSession::status() const noexcept { return status_; }

QList<ControllerFeedbackState> HostedCoreSession::controllerFeedback() const {
    if (status_.controllerFeedback.isEmpty() || hostProcessState() != HostProcessState::Running) return {};
    return status_.controllerFeedback;
}

CoreRuntimeStatus HostedCoreSession::coreRuntimeStatus() const noexcept {
    CoreRuntimeStatus result;
    result.supportsSaveStates = status_.supportsSaveStates;
    result.region = status_.runtime.region;
    result.bufferedAudioFrames = std::max(0, status_.runtime.bufferedAudioFrames);
    result.audioOverrunCount = quint64(std::max<qint64>(0, status_.runtime.audioOverruns));
    result.audioUnderrunCount = quint64(std::max<qint64>(0, status_.runtime.audioUnderruns));
    result.hostProcessState = hostProcessState();
    result.hostProcessId = hostProcessId();
    return result;
}
const QStringList& HostedCoreSession::diagnostics() const noexcept { return diagnostics_; }
std::shared_ptr<const VideoFrame> HostedCoreSession::latestFrame() const noexcept {
    return videoOutput_ ? videoOutput_->latest : nullptr;
}

QList<std::shared_ptr<const VideoFrame>> HostedCoreSession::previousVideoFields(const VideoFrame& frame) const {
    if (!videoOutput_ || videoOutput_->latest.get() != &frame) return {};
    return videoOutput_->previous;
}

namespace HostProtocol {
static bool writeInitializationDirectories(QIODevice& device, const QString& sessionDirectory,
    const QString& saveDirectory, const QJsonObject& configuration) {
    return writeString(device, QDir(sessionDirectory).absolutePath()) &&
        writeOptionalString(device, saveDirectory.isNull() ? QString{} : QDir(saveDirectory).absolutePath()) &&
        writeString(device, QString::fromUtf8(QJsonDocument(configuration).toJson(QJsonDocument::Compact)));
}

bool writeInitialization(QIODevice& device, const QString& corePath, qint32 emulator, const QString& sessionDirectory,
    const QString& saveDirectory, const QJsonObject& configuration) {
    return writeString(device, corePath) && writeIndex(device, emulator) &&
        writeInitializationDirectories(device, sessionDirectory, saveDirectory, configuration);
}

bool writeInitialization(QIODevice& device, const QString& corePath, const QString& sessionDirectory,
    const QString& saveDirectory, const QJsonObject& configuration) {
    return writeString(device, corePath) && writeInitializationDirectories(device, sessionDirectory, saveDirectory, configuration);
}
}
}
