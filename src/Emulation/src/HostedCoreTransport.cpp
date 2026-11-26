#include <RetroMediaLab/Emulation/HostedCoreTransport.h>
#include <QBuffer>
#include <QDataStream>
#include <QDeadlineTimer>
#include <QFileInfo>
#include <QJsonDocument>
#include <QLocalServer>
#include <QLocalSocket>
#include <QProcess>
#include <QStandardPaths>
#include <QtEndian>
#include <algorithm>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace RetroMediaLab::Emulation {
namespace {
constexpr int WaitSliceMs = 50;
constexpr int PipeBufferSize = 64 * 1024;
constexpr auto PipePrefix = "rml-core-";
constexpr auto VideoPrefix = "rml-v-";
constexpr quint8 SuccessResponse = 1;
constexpr quint8 FailureResponse = 2;
constexpr qsizetype HeaderSize = sizeof(qint32) + sizeof(quint8);

int waitSlice(const QDeadlineTimer& deadline, std::stop_token token) {
    return token.stop_requested() ? 0 : static_cast<int>(std::clamp(deadline.remainingTime(), qint64{}, qint64{WaitSliceMs}));
}

bool readExact(QLocalSocket& socket, char* target, qint64 size, const QDeadlineTimer& deadline, std::stop_token token) {
    qint64 offset = 0;
    while (offset < size) {
        const auto wait = waitSlice(deadline, token);
        if (wait <= 0) return false;
        if (socket.bytesAvailable() > 0) {
            const auto read = socket.read(target + offset, size - offset);
            if (read <= 0) return false;
            offset += read;
        } else {
            if (socket.state() != QLocalSocket::ConnectedState) return false;
            socket.waitForReadyRead(wait);
        }
    }
    return true;
}

bool writeRequest(QLocalSocket& socket, QByteArrayView request, const QDeadlineTimer& deadline, std::stop_token token) {
    qsizetype offset = 0;
    while (offset < request.size()) {
        if (waitSlice(deadline, token) <= 0 || socket.state() != QLocalSocket::ConnectedState) return false;
        const auto count = std::min<qsizetype>(PipeBufferSize, request.size() - offset);
        const auto written = socket.write(request.data() + offset, count);
        if (written <= 0) return false;
        offset += static_cast<qsizetype>(written);
        while (socket.bytesToWrite() > 0) {
            const auto wait = waitSlice(deadline, token);
            if (wait <= 0 || socket.state() != QLocalSocket::ConnectedState) return false;
            socket.waitForBytesWritten(wait);
        }
    }
    return true;
}

std::optional<QByteArray> readResponse(QLocalSocket& socket, const QDeadlineTimer& deadline, std::stop_token token) {
    char header[sizeof(qint32)]{};
    if (!readExact(socket, header, sizeof(header), deadline, token)) return {};
    const auto length = qFromLittleEndian<qint32>(header);
    if (length < 0 || length > HostProtocol::MaximumBlobLength) return {};
    QByteArray result("");
    for (qint32 offset = 0; offset < length;) {
        const auto count = std::min(PipeBufferSize, length - offset);
        result.resize(offset + count);
        if (!readExact(socket, result.data() + offset, count, deadline, token)) return {};
        offset += count;
    }
    return result;
}

#ifdef Q_OS_WIN
struct HandleCloser {
    void operator()(HANDLE handle) const noexcept { CloseHandle(handle); }
};
using UniqueHandle = std::unique_ptr<void, HandleCloser>;

UniqueHandle createProcessJob() {
    UniqueHandle job(CreateJobObjectW(nullptr, nullptr));
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
    limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if (!job || !SetInformationJobObject(job.get(), JobObjectExtendedLimitInformation, &limits, sizeof(limits))) return {};
    return job;
}

bool assignProcessJob(HANDLE job, qint64 pid) {
    UniqueHandle process(OpenProcess(PROCESS_SET_QUOTA | PROCESS_TERMINATE, FALSE, static_cast<DWORD>(pid)));
    return process && AssignProcessToJobObject(job, process.get());
}
#endif
}

struct HostedCoreTransport::Data {
    explicit Data(int gracefulTimeout) : gracefulTimeout(gracefulTimeout) {}
    ~Data() {
        if (socket) {
            socket->abort();
            socket.reset();
        }
        if (process.state() == QProcess::NotRunning) return;
        if (graceful && process.waitForFinished(gracefulTimeout)) return;
        process.kill();
#ifdef Q_OS_WIN
        job.reset();
#endif
        process.waitForFinished(gracefulTimeout);
    }

    QProcess process;
    std::unique_ptr<QLocalSocket> socket;
    std::unique_ptr<HostProtocol::SharedVideoMemory> video;
#ifdef Q_OS_WIN
    UniqueHandle job;
#endif
    const int gracefulTimeout;
    bool graceful = false;
};

HostedCoreTransport::HostedCoreTransport(std::unique_ptr<Data> data) : data_(std::move(data)) {}
HostedCoreTransport::~HostedCoreTransport() = default;

std::unique_ptr<HostedCoreTransport> HostedCoreTransport::start(const HostedCoreStartOptions& options, std::stop_token token) {
    qint64 startedProcessId;
    return start(options, startedProcessId, token);
}

std::unique_ptr<HostedCoreTransport> HostedCoreTransport::start(const HostedCoreStartOptions& options,
        qint64& startedProcessId, std::stop_token token) {
    startedProcessId = 0;
    if (token.stop_requested() || options.executable.isEmpty() || options.command.isEmpty() ||
        options.connectionTimeoutMs <= 0 || options.gracefulShutdownTimeoutMs < 0) return {};
    // Keep keys below the limit of the native shared-memory backend on macOS.
    constexpr qsizetype NameEntropyCharacters = 22;
    const auto id = QUuid::createUuid().toString(QUuid::Id128).right(NameEntropyCharacters);
    const auto pipeName = QLatin1String(PipePrefix) + id;
    const auto videoName = QLatin1String(VideoPrefix) + id;
    auto data = std::make_unique<Data>(options.gracefulShutdownTimeoutMs);
    data->video = options.videoMapping == HostedVideoMapping::Resizable
        ? HostProtocol::SharedVideoMemory::resizableReader(videoName, options.minimumVideoSlotCapacity)
        : HostProtocol::SharedVideoMemory::createReader(videoName);
    if (!data->video) return {};

    QLocalServer server;
    server.setSocketOptions(QLocalServer::UserAccessOption);
    server.setMaxPendingConnections(1);
    if (!server.listen(pipeName)) return {};

    auto& process = data->process;
    auto executable = options.executable;
    auto directory = options.workingDirectory;
    if (directory.isEmpty()) {
        const QFileInfo localExecutable(executable);
        executable = localExecutable.isFile() ? localExecutable.absoluteFilePath()
            : QStandardPaths::findExecutable(executable);
        if (executable.isEmpty()) return {};
        directory = QFileInfo(executable).absolutePath();
    }
    process.setProgram(executable);
    process.setWorkingDirectory(directory);
    process.setArguments({options.command, pipeName, videoName});
    process.setStandardOutputFile(QProcess::nullDevice());
    process.setStandardErrorFile(QProcess::nullDevice());
#ifdef Q_OS_WIN
    data->job = createProcessJob();
    if (!data->job) return {};
    process.setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments* args) { args->flags |= CREATE_NO_WINDOW; });
#endif
    const QDeadlineTimer deadline(options.connectionTimeoutMs, Qt::PreciseTimer);
    process.start();
    while (process.state() == QProcess::Starting) {
        const auto wait = waitSlice(deadline, token);
        if (wait <= 0) return {};
        process.waitForStarted(wait);
    }
    if (process.state() != QProcess::Running) return {};
    startedProcessId = process.processId();
#ifdef Q_OS_WIN
    if (!assignProcessJob(data->job.get(), startedProcessId)) return {};
#endif
    process.closeWriteChannel();
    while (!server.hasPendingConnections()) {
        const auto wait = waitSlice(deadline, token);
        if (wait <= 0 || process.waitForFinished(0)) return {};
        bool timedOut = false;
        if (!server.waitForNewConnection(wait, &timedOut) && !timedOut && !server.hasPendingConnections()) return {};
    }
    if (token.stop_requested()) return {};
    data->socket.reset(server.nextPendingConnection());
    if (!data->socket || data->socket->state() != QLocalSocket::ConnectedState) return {};
    data->socket->setParent(nullptr);
    data->socket->setReadBufferSize(PipeBufferSize);
    data->graceful = true;
    return std::unique_ptr<HostedCoreTransport>(new HostedCoreTransport(std::move(data)));
}

std::optional<QByteArray> HostedCoreTransport::exchange(QByteArrayView request, int timeoutMs, std::stop_token token) {
    if (!connected() || request.isEmpty() || request.size() > HostProtocol::MaximumBlobLength ||
        timeoutMs <= 0 || token.stop_requested()) return {};
    const QDeadlineTimer deadline(timeoutMs, Qt::PreciseTimer);
    auto& socket = *data_->socket;
    if (writeRequest(socket, request, deadline, token)) {
        auto payload = readResponse(socket, deadline, token);
        if (payload) return payload;
    }
    socket.abort();
    data_->graceful = false;
    return {};
}

std::optional<HostedCoreReply> HostedCoreTransport::command(qint32 version, HostedCoreCommand command,
    QByteArrayView payload, int timeoutMs, std::stop_token token) {
    if (payload.size() > HostProtocol::MaximumBlobLength - HeaderSize) return {};
    QByteArray request;
    request.reserve(HeaderSize + payload.size());
    QBuffer output(&request);
    if (!output.open(QIODevice::WriteOnly) ||
        !HostProtocol::writeHeader(output, version, static_cast<quint8>(command)) ||
        (!payload.isEmpty() && output.write(payload.data(), payload.size()) != payload.size())) return {};
    auto response = exchange(request, timeoutMs, token);
    if (!response) return {};

    QBuffer input(&*response);
    if (!input.open(QIODevice::ReadOnly)) return {};
    const auto status = HostProtocol::readHeader(input, version);
    if (status == SuccessResponse) return HostedCoreReply{input.readAll(), std::nullopt};
    if (status == FailureResponse) {
        const auto error = HostProtocol::readString(input);
        if (error && input.atEnd()) {
            const auto diagnostic = QJsonDocument::fromJson(error->toUtf8());
            if (diagnostic.isObject()) return HostedCoreReply{{}, diagnostic.object()};
        }
    }
    data_->socket->abort();
    data_->graceful = false;
    return {};
}

bool HostedCoreTransport::connected() const noexcept {
    return data_->socket && data_->socket->state() == QLocalSocket::ConnectedState;
}

bool HostedCoreTransport::isProcessRunning() const noexcept {
    auto& process = data_->process;
    return process.state() == QProcess::Running && !process.waitForFinished(0) && process.state() == QProcess::Running;
}

qint64 HostedCoreTransport::processId() const noexcept {
    return data_->process.processId();
}

HostProtocol::SharedVideoMemory& HostedCoreTransport::videoMemory() noexcept {
    return *data_->video;
}

namespace HostProtocol {
bool writeMediaSlot(QIODevice& device, HostedMediaSlot slot) {
    if (slot.category < HostedMediaCategory::FloppyDrive || slot.category > HostedMediaCategory::MemoryCard ||
        slot.index < 0) return false;
    QDataStream stream(&device);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << static_cast<qint32>(slot.category) << slot.index;
    return stream.status() == QDataStream::Ok;
}

bool writeOption(QIODevice& device, const QString& id, const QString& value) {
    return writeString(device, id) && writeString(device, value);
}

bool writeIndex(QIODevice& device, qint32 index) {
    QDataStream stream(&device);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << index;
    return stream.status() == QDataStream::Ok;
}

bool writeControllerPortDevice(QIODevice& device, qint32 port, qint32 peripheral) {
    QDataStream stream(&device);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << port << peripheral;
    return stream.status() == QDataStream::Ok;
}

std::optional<bool> readMediaChanges(QIODevice& device) {
    const auto value = device.read(sizeof(quint8));
    if (value.size() != sizeof(quint8)) return {};
    return value.front() != 0;
}
}
}
