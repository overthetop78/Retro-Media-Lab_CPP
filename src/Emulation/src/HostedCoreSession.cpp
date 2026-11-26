#include <RetroMediaLab/Emulation/HostedCoreSession.h>
#include <RetroMediaLab/Emulation/StorageSettings.h>
#include <QBuffer>
#include <QDataStream>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QSet>
#include <algorithm>
#include <limits>
#include <stdexcept>

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
constexpr auto MachineStartFailed = "Emulation.Error.MachineStartFailed";

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

QJsonValue nullableDiskText(const QString& text) {
    return text.isNull() ? QJsonValue(QJsonValue::Null) : QJsonValue(text);
}

bool writeDiskReport(QIODevice& response, const HostedCoreDiskReport& report) {
    QJsonArray images;
    for (const auto& image : report.images) {
        images.append(QJsonObject{{QLatin1String(DiskImageIndexKey), image.index},
            {QLatin1String(MediaPathKey), nullableDiskText(image.path)},
            {QLatin1String(DiskImageLabelKey), nullableDiskText(image.label)}});
    }
    const QJsonObject object{{QLatin1String(DiskImageCountKey), report.imageCount},
        {QLatin1String(DiskCurrentIndexKey), report.currentIndex},
        {QLatin1String(DiskEjectedKey), report.ejected}, {QLatin1String(DiskImagesKey), images}};
    return HostProtocol::writeString(response, QString::fromUtf8(QJsonDocument(object).toJson(QJsonDocument::Compact)));
}

std::optional<qint32> readIndex(QIODevice& input) {
    qint32 value{};
    QDataStream stream(&input);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream >> value;
    if (stream.status() != QDataStream::Ok) return {};
    return value;
}

std::optional<QJsonObject> readJsonObject(QIODevice& input) {
    const auto text = HostProtocol::readString(input);
    if (!text) return {};
    const auto document = QJsonDocument::fromJson(text->toUtf8());
    if (!document.isObject()) return {};
    return document.object();
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

bool writeTiming(QIODevice& output, const HostedCoreStatus& status) {
    QDataStream stream(&output);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::DoublePrecision);
    stream << status.framesPerSecond << status.sampleRate << status.minimumAudioLatencyMs;
    return stream.status() == QDataStream::Ok;
}

bool writeJsonArray(QIODevice& output, const QJsonArray& array) {
    return HostProtocol::writeString(output, QString::fromUtf8(QJsonDocument(array).toJson(QJsonDocument::Compact)));
}

bool writeStatusTail(QIODevice& output, const HostedCoreStatus& status, bool shutdownAndFeedback) {
    return HostProtocol::writeLedStates(output, status.leds) && (!shutdownAndFeedback ||
        (output.putChar(status.shutdownRequested ? char{1} : char{0}) &&
            HostProtocol::writeControllerFeedback(output, status.controllerFeedback)));
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
bool executeHostedCoreCommand(HostedCoreCommand command, IHostedCoreCommands& core,
        QIODevice& request, QIODevice& response) {
    switch (command) {
    case HostedCoreCommand::HardReset: core.hardReset(); return true;
    case HostedCoreCommand::SoftReset: core.softReset(); return true;
    case HostedCoreCommand::Stop: core.stop(); return true;
    case HostedCoreCommand::InsertMedia: {
        const auto media = readJsonObject(request);
        if (!media) return false;
        core.insertMedia(*media);
        return true;
    }
    case HostedCoreCommand::EjectMedia:
    case HostedCoreCommand::SaveMediaChanges:
    case HostedCoreCommand::HasUnsavedMediaChanges: {
        const auto slot = readMediaSlot(request);
        if (!slot) return false;
        if (command == HostedCoreCommand::EjectMedia) core.ejectMedia(*slot);
        else if (command == HostedCoreCommand::SaveMediaChanges) core.saveMediaChanges(*slot);
        else return response.putChar(core.hasUnsavedMediaChanges(*slot) ? char{1} : char{0});
        return true;
    }
    case HostedCoreCommand::GetDiskStatus: return writeDiskReport(response, core.diskStatus());
    case HostedCoreCommand::SaveState: return writeBytes(response, core.saveState());
    case HostedCoreCommand::LoadState: {
        const auto state = readBytes(request);
        if (!state) return false;
        core.loadState(*state);
        return true;
    }
    case HostedCoreCommand::SetOption: {
        const auto id = readString(request);
        if (!id) return false;
        const auto value = readString(request);
        if (!value) return false;
        core.setOption(*id, *value);
        return true;
    }
    case HostedCoreCommand::SelectDisk: {
        const auto index = readIndex(request);
        if (!index) return false;
        core.selectDisk(*index);
        return true;
    }
    case HostedCoreCommand::SetControllerPortDevice: {
        const auto port = readIndex(request);
        if (!port) return false;
        const auto device = readIndex(request);
        if (!device) return false;
        core.setControllerPortDevice(*port, *device);
        return true;
    }
    default: return false;
    }
}

std::optional<HostedCoreInitialization> readInitialization(QIODevice& input, bool hasEmulator) {
    auto path = readString(input);
    if (!path) return {};
    const auto emulator = hasEmulator ? readIndex(input) : std::optional<qint32>{};
    if (hasEmulator && !emulator) return {};
    auto session = readString(input);
    if (!session) return {};
    auto saves = readOptionalString(input);
    if (!saves) return {};
    auto configuration = readJsonObject(input);
    if (!configuration) return {};
    return HostedCoreInitialization{std::move(*path), emulator, std::move(*session), std::move(*saves),
        std::move(*configuration)};
}

std::optional<HostedCoreFrameRequest> readHostedFrameRequest(QIODevice& input) {
    auto keys = readInput(input);
    std::optional<AudioBufferStatus> status;
    if (!keys || !readAudioBufferStatus(input, status)) return {};
    return HostedCoreFrameRequest{std::move(*keys), status};
}

HostedCoreReplyWriter::HostedCoreReplyWriter(const HostedCoreSessionOptions& layout)
    : frameStateSupport_(layout.frameStateSupport), frameDiskSelection_(layout.frameDiskSelection),
      shutdownAndFeedback_(layout.shutdownAndFeedback), initializationDiskSelection_(layout.initializationDiskSelection) {}

bool HostedCoreReplyWriter::writeInitializationReply(QIODevice& output, const HostedCoreInfo& info,
        const HostedCoreStatus& status, const QStringList& diagnostics) {
    if (initializationDiskSelection_ && !status.diskSelection) return false;
    auto extensions = info.contentExtensions;
    std::stable_sort(extensions.begin(), extensions.end(), [](const QString& first, const QString& second) {
        return first.compare(second, Qt::CaseInsensitive) < 0;
    });
    if (!writeString(output, info.sha256) || !writeTiming(output, status) ||
        !output.putChar(status.supportsSaveStates ? char{1} : char{0}) || !writeRuntimeStatus(output, status.runtime) ||
        !writeJsonArray(output, info.options) || !writeJsonArray(output, QJsonArray::fromStringList(diagnostics)) ||
        !writeString(output, info.name) || !writeString(output, info.version) ||
        !writeString(output, extensions.join(ExtensionSeparator)) ||
        (initializationDiskSelection_ && !writeDiskSelection(output, *status.diskSelection)) ||
        !writeStatusTail(output, status, shutdownAndFeedback_)) return false;
    lastDiagnosticCount_ = diagnostics.size();
    return true;
}

bool HostedCoreReplyWriter::writeFrameReply(QIODevice& output, SharedVideoMemory& video, const VideoFrame* frame,
        const QList<AudioChunk>& audio, const HostedCoreStatus& status, const QStringList& diagnostics) {
    if (frameDiskSelection_ && !status.diskSelection) return false;
    const auto* nextFrame = frame && frame->sequence != lastVideoSequence_ ? frame : nullptr;
    const bool diagnosticsChanged = diagnostics.size() != lastDiagnosticCount_;
    if (!video.writeFrame(output, nextFrame) || !writeAudio(output, audio) || !writeTiming(output, status) ||
        !writeRuntimeStatus(output, status.runtime) ||
        (frameStateSupport_ && !output.putChar(status.supportsSaveStates ? char{1} : char{0})) ||
        (frameDiskSelection_ && !writeDiskSelection(output, *status.diskSelection)) ||
        !output.putChar(diagnosticsChanged ? char{1} : char{0}) ||
        (diagnosticsChanged && !writeJsonArray(output, QJsonArray::fromStringList(diagnostics))) ||
        !writeStatusTail(output, status, shutdownAndFeedback_)) return false;
    if (frame) lastVideoSequence_ = frame->sequence;
    lastDiagnosticCount_ = diagnostics.size();
    return true;
}

bool runHostedCore(const QString& pipeName, const QString& videoMapName, const HostedCoreSessionOptions& options,
        bool hasEmulator, const HostedCoreFactory& createCore, const HostedCoreErrorDescription& describeError,
        std::stop_token token) {
    if (pipeName.isEmpty() || videoMapName.isEmpty() || !createCore || !describeError || token.stop_requested())
        return false;
    auto video = options.transport.videoMapping == HostedVideoMapping::Resizable
        ? SharedVideoMemory::resizableWriter(videoMapName, options.transport.minimumVideoSlotCapacity)
        : SharedVideoMemory::openWriter(videoMapName);
    if (!video) return false;
    HostedCoreReplyWriter writer(options);
    std::unique_ptr<IHostedCoreRuntime> core;
    const auto activeCore = [&]() -> IHostedCoreRuntime& {
        if (!core) throw std::runtime_error(MachineStartFailed);
        return *core;
    };
    const auto execute = [&](HostedCoreCommand command, QIODevice& input, QIODevice& output) {
        switch (command) {
        case HostedCoreCommand::Initialize: {
            const auto request = readInitialization(input, hasEmulator);
            if (!request) return false;
            core.reset();
            auto candidate = createCore(*request);
            if (!candidate) throw std::runtime_error(MachineStartFailed);
            candidate->initialize(*request);
            core = std::move(candidate);
            return writer.writeInitializationReply(output, core->info(), core->status(), core->diagnostics());
        }
        case HostedCoreCommand::RunFrame: {
            const auto request = readHostedFrameRequest(input);
            if (!request) return false;
            auto& running = activeCore();
            running.setInput(request->input);
            running.setAudioBufferStatus(request->audioStatus);
            running.runFrame();
            const auto frame = running.latestFrame();
            const auto status = running.status();
            const auto audio = running.takeAudio();
            return writer.writeFrameReply(output, *video, frame.get(), audio, status, running.diagnostics());
        }
        case HostedCoreCommand::Dispose:
            core.reset();
            return true;
        default:
            return executeHostedCoreCommand(command, activeCore(), input, output);
        }
    };
    return runHostedCoreConnection(pipeName, options.version, options.transport.connectionTimeoutMs, execute,
        [&](std::exception_ptr error) { return describeError(error, core ? core->diagnostics() : QStringList{}); }, token);
}

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
