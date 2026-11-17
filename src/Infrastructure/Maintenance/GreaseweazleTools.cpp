#include "GreaseweazleTools.h"
#include "Media/MediaCommands.h"
#include "GwToolsConstants.h"
#include "Logging/ConsoleLogSession.h"
#include "Logging/DiagnosticLog.h"
#include <QFileInfo>
#include <QProcess>
#include <QTimer>
#include <QStringDecoder>
#include <QRegularExpression>
#include <vector>
#include <algorithm>
#include <stdexcept>
#ifdef Q_OS_WIN
#include <windows.h>
#else
#include <signal.h>
#endif
namespace RetroMediaLab {
using namespace GwToolsConstants;
namespace {
[[noreturn]] void invalid() { throw std::invalid_argument(InvalidKey); }
QString required(const QMap<QString,QString>& values, const QString& key) { const auto value = values.value(key).trimmed(); if (value.isEmpty()) invalid(); return value; }
int integer(const QString& text, bool positive = false) {
    bool ok = false; const auto result = text.toInt(&ok);
    if (!ok || !QRegularExpression(IntegerPattern).match(text.trimmed()).hasMatch() || result < 0 || (positive && result == 0)) invalid();
    return result;
}
#ifdef Q_OS_WIN
struct ProcessJob {
    HANDLE job = nullptr;
    STARTUPINFOEXW startup{};
    std::vector<unsigned char> storage;
    ProcessJob() {
        job = CreateJobObjectW(nullptr, nullptr); if (!job) throw std::runtime_error(StartFailedKey);
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{}; limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        if (!SetInformationJobObject(job, JobObjectExtendedLimitInformation, &limits, sizeof(limits))) { CloseHandle(job); job = nullptr; throw std::runtime_error(StartFailedKey); }
    }
    ~ProcessJob() { if (startup.lpAttributeList) DeleteProcThreadAttributeList(startup.lpAttributeList); if (job) CloseHandle(job); }
    void prepare(QProcess::CreateProcessArguments* arguments) {
        SIZE_T length = 0; InitializeProcThreadAttributeList(nullptr, 1, 0, &length); storage.resize(length);
        auto* attributes = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(storage.data());
        if (!InitializeProcThreadAttributeList(attributes, 1, 0, &length)) throw std::runtime_error(StartFailedKey);
        startup.lpAttributeList = attributes;
        if (!UpdateProcThreadAttribute(attributes, 0, PROC_THREAD_ATTRIBUTE_JOB_LIST, &job, sizeof(job), nullptr, nullptr)) throw std::runtime_error(StartFailedKey);
        startup.StartupInfo = *arguments->startupInfo; startup.StartupInfo.cb = sizeof(startup);
        arguments->startupInfo = &startup.StartupInfo; arguments->flags |= CREATE_NO_WINDOW | EXTENDED_STARTUPINFO_PRESENT;
    }
};
#endif
}
const QVector<GwToolDefinition>& gwToolDefinitions() {
    static const QVector<GwToolDefinition> definitions{
        {Info, {}},
        {Bandwidth, {}},
        {Rpm, {{NrParameter, NrParameterLabel, NrParameterDefault, GwParameterKind::Required}}},
        {Seek, {{CylinderParameter, CylinderParameterLabel, CylinderParameterDefault, GwParameterKind::Required}, {ForceParameter, ForceParameterLabel, ForceParameterDefault, GwParameterKind::Flag}, {MotorOnParameter, MotorOnParameterLabel, MotorOnParameterDefault, GwParameterKind::Flag}}},
        {Align, {{TracksParameter, TracksParameterLabel, TracksParameterDefault, GwParameterKind::Required}, {RevsParameter, RevsParameterLabel, RevsParameterDefault, GwParameterKind::Required}, {ReadsParameter, ReadsParameterLabel, ReadsParameterDefault, GwParameterKind::Required}, {FormatParameter, FormatParameterLabel, FormatParameterDefault, GwParameterKind::Optional}, {DiskdefsParameter, DiskdefsParameterLabel, DiskdefsParameterDefault, GwParameterKind::Optional}, {RawParameter, RawParameterLabel, RawParameterDefault, GwParameterKind::Flag}, {FakeIndexParameter, FakeIndexParameterLabel, FakeIndexParameterDefault, GwParameterKind::Optional}, {HardSectorsParameter, HardSectorsParameterLabel, HardSectorsParameterDefault, GwParameterKind::Flag}, {AdjustSpeedParameter, AdjustSpeedParameterLabel, AdjustSpeedParameterDefault, GwParameterKind::Optional}, {PllParameter, PllParameterLabel, PllParameterDefault, GwParameterKind::Optional}, {DenselParameter, DenselParameterLabel, DenselParameterDefault, GwParameterKind::Optional}, {GenTg43Parameter, GenTg43ParameterLabel, GenTg43ParameterDefault, GwParameterKind::Flag}, {ReverseParameter, ReverseParameterLabel, ReverseParameterDefault, GwParameterKind::Flag}}},
        {Pin, {{PinParameter, PinParameterLabel, PinParameterDefault, GwParameterKind::Required}, {SetParameter, SetParameterLabel, SetParameterDefault, GwParameterKind::Flag}, {HighParameter, HighParameterLabel, HighParameterDefault, GwParameterKind::Flag}}},
        {Reset, {}},
        {Delays, {{SelectParameter, SelectParameterLabel, SelectParameterDefault, GwParameterKind::Optional}, {StepParameter, StepParameterLabel, StepParameterDefault, GwParameterKind::Optional}, {SettleParameter, SettleParameterLabel, SettleParameterDefault, GwParameterKind::Optional}, {MotorParameter, MotorParameterLabel, MotorParameterDefault, GwParameterKind::Optional}, {WatchdogParameter, WatchdogParameterLabel, WatchdogParameterDefault, GwParameterKind::Optional}, {PreWriteParameter, PreWriteParameterLabel, PreWriteParameterDefault, GwParameterKind::Optional}, {PostWriteParameter, PostWriteParameterLabel, PostWriteParameterDefault, GwParameterKind::Optional}, {IndexMaskParameter, IndexMaskParameterLabel, IndexMaskParameterDefault, GwParameterKind::Optional}}},
        {Update, {{BootloaderParameter, BootloaderParameterLabel, BootloaderParameterDefault, GwParameterKind::Flag}}}
    };
    return definitions;
}
QString GwToolCommand::displayText() const {
    const auto quote = [](QString value) { if (value.isEmpty() || value.contains(QRegularExpression(WhiteSpace)) || value.contains(Quote)) { value.replace(Quote, EscapedQuote); return QString(Quote) + value + Quote; } return value; };
    QStringList values{quote(executable)}; for (const auto& arg : arguments) values.append(quote(arg)); return values.join(Space);
}
GwToolCommand GreaseweazleTools::build(const QString& verb, const QMap<QString,QString>& values, const QSet<QString>& enabled, const QString& executable, const QString& device, const QString& drive) {
    const auto& definitions = gwToolDefinitions(); const auto found = std::find_if(definitions.begin(), definitions.end(), [&](const auto& definition) { return verb == definition.id; }); if (found == definitions.end()) invalid();
    for (const auto& field : enabled) if (std::none_of(found->parameters.begin(), found->parameters.end(), [&](const auto& p) { return field == p.id && p.kind != GwParameterKind::Required; })) invalid();
    if (executable.contains(QChar::Null)) invalid();
    QStringList args{verb};
    const auto number = [&](const char* key, bool positive = false) { return QString::number(integer(required(values, key), positive)); };
    const auto add = [&](const QString& key, const QString& value) { args << QString(Prefix) + key << value; };
    if (verb == Rpm) add(NrParameter, number(NrParameter, true));
    else if (verb == Seek) { args << number(CylinderParameter); for (const auto* flag : {ForceParameter, MotorOnParameter}) if (enabled.contains(flag)) args << QString(Prefix) + flag; }
    else if (verb == Pin) { const auto pin = integer(required(values, PinParameter)); if (pin != Pin8 && pin != Pin26 && pin != Pin28) invalid(); const bool set = enabled.contains(SetParameter); args << (set ? Set : Get) << QString::number(pin); if (set) args << (enabled.contains(HighParameter) ? High : Low); }
    else if (verb == Delays) { for (const auto& parameter : found->parameters) if (enabled.contains(parameter.id)) add(parameter.id, number(parameter.id)); }
    else if (verb == Align) {
        if ((enabled.contains(FakeIndexParameter) && enabled.contains(HardSectorsParameter)) || (enabled.contains(DenselParameter) && enabled.contains(GenTg43Parameter))) invalid();
        const auto spec = required(values, TracksParameter); Infrastructure::validateGwTrackSpec(spec); add(TracksParameter, spec); add(RevsParameter, number(RevsParameter, true)); add(ReadsParameter, number(ReadsParameter, true));
        for (const auto* key : {FormatParameter, DiskdefsParameter, FakeIndexParameter, AdjustSpeedParameter, PllParameter, DenselParameter}) if (enabled.contains(key)) {
            const auto value = required(values, key);
            if (QString(key) == DenselParameter && value != High && value != Low) invalid();
            if (QString(key) == PllParameter) Infrastructure::validateGwPllSpec(value);
            if (QString(key) == FakeIndexParameter || QString(key) == AdjustSpeedParameter) { const auto match = QRegularExpression(SpeedPattern).match(value); bool ok = false; const auto speed = match.captured(1).toDouble(&ok); if (!match.hasMatch() || !ok || !qIsFinite(speed) || speed <= 0) invalid(); }
            add(key, value);
        }
        for (const auto* flag : {RawParameter, HardSectorsParameter, GenTg43Parameter, ReverseParameter}) if (enabled.contains(flag)) args << QString(Prefix) + flag;
    } else if (verb == Update && enabled.contains(BootloaderParameter)) args << QString(Prefix) + BootloaderParameter;
    if (!device.trimmed().isEmpty()) args << DeviceOption << device;
    if (!drive.trimmed().isEmpty() && (verb == Rpm || verb == Seek || verb == Align)) args << DriveOption << drive;
    for (const auto& arg : args) if (arg.contains(QChar::Null)) invalid();
    return {executable, args};
}
struct GreaseweazleTools::Data {
    QMap<QString,GwToolDraft> drafts;
    QString active, directory;
    bool cancelled = false, destroying = false;
    qint64 group = 0;
    std::function<LoggingSettings()> logging;
    std::function<QString(const QString&)> translate;
    std::vector<std::weak_ptr<const std::function<void()>>> observers;
    QProcess process;
    QTimer stopTimer;
    QStringDecoder decoder{QStringDecoder::Utf8};
    std::unique_ptr<ConsoleLogSession> log;
#ifdef Q_OS_WIN
    std::unique_ptr<ProcessJob> job;
#endif
};
GreaseweazleTools::GreaseweazleTools(QString directory, std::function<LoggingSettings()> logging, std::function<QString(const QString&)> translate)
    : data_(std::make_unique<Data>()) {
    data_->directory = std::move(directory); data_->logging = std::move(logging); data_->translate = std::move(translate);
    for (const auto& tool : gwToolDefinitions()) { GwToolDraft draft; draft.summaryKey = ReadyKey; for (const auto& parameter : tool.parameters) if (parameter.kind != GwParameterKind::Flag) draft.values.insert(parameter.id, parameter.initial); data_->drafts.insert(tool.id, draft); }
    for (const auto* operation : {Read, Write, Convert}) { GwToolDraft draft; draft.summaryKey = ReadyKey; data_->drafts.insert(operation, draft); }
    data_->process.setProcessChannelMode(QProcess::MergedChannels); data_->stopTimer.setSingleShot(true);
    connect(&data_->process, &QProcess::readyReadStandardOutput, this, [this] { output(); });
    connect(&data_->process, &QProcess::started, this, [this] { data_->group = data_->process.processId(); });
    connect(&data_->process, &QProcess::finished, this, [this](int code, QProcess::ExitStatus status) { finish(code, status == QProcess::NormalExit); });
    connect(&data_->process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) { if (error == QProcess::FailedToStart) finish(0, false, true); });
    connect(&data_->stopTimer, &QTimer::timeout, this, [this] {
#ifdef Q_OS_WIN
        data_->job.reset();
#else
        if (data_->group > 0) ::kill(-data_->group, SIGKILL);
#endif
        data_->process.kill();
    });
}
GreaseweazleTools::~GreaseweazleTools() {
    data_->destroying = true; data_->stopTimer.stop(); data_->process.disconnect(this);
#ifdef Q_OS_WIN
    data_->job.reset();
#else
    if (data_->group > 0) ::kill(-data_->group, SIGKILL);
#endif
    if (data_->process.state() != QProcess::NotRunning) { data_->process.kill(); data_->process.waitForFinished(InfiniteWait); }
}
const GwToolDraft& GreaseweazleTools::draft(const QString& verb) const { const auto found = data_->drafts.constFind(verb); if (found == data_->drafts.cend()) invalid(); return *found; }
void GreaseweazleTools::setValue(const QString& verb, const QString& field, const QString& value) { if (verb == data_->active || !draft(verb).values.contains(field)) return; data_->drafts[verb].values[field] = value; data_->drafts[verb].summaryKey = ReadyKey; data_->drafts[verb].information = false; publish(); }
void GreaseweazleTools::setEnabled(const QString& verb, const QString& field, bool enabled) { if (verb == data_->active) return; draft(verb); if (enabled) data_->drafts[verb].enabled.insert(field); else data_->drafts[verb].enabled.remove(field); data_->drafts[verb].summaryKey = ReadyKey; data_->drafts[verb].information = false; publish(); }
GwToolCommand GreaseweazleTools::command(const QString& verb, const HardwareSettings& hardware, const QString& id) const {
    const auto found = std::find_if(hardware.devices.begin(), hardware.devices.end(), [&](const auto& device) { return device.id == id && device.backend == HardwareConstants::Greaseweazle && device.category == HardwareConstants::Floppy; });
    if (found == hardware.devices.end() || !found->available) throw std::runtime_error(NoReaderKey);
    if (!QFileInfo(hardware.gwPath).isFile()) throw std::runtime_error(HardwareConstants::ToolMissingKey);
    const auto& current = draft(verb); return build(verb, current.values, current.enabled, hardware.gwPath, found->address, found->selection);
}
void GreaseweazleTools::execute(const QString& verb, const HardwareSettings& hardware, const QString& id) {
    executeCommand(verb, command(verb, hardware, id));
}
void GreaseweazleTools::executeCommand(const QString& verb, const GwToolCommand& built) {
    if (!data_->active.isEmpty()) throw std::runtime_error(BusyKey);
    draft(verb);
    if (!QFileInfo(built.executable).isFile() || built.executable.contains(QChar::Null)) throw std::runtime_error(StartFailedKey);
    for (const auto& argument : built.arguments) if (argument.contains(QChar::Null)) invalid();
    try {
#ifdef Q_OS_WIN
    data_->job = std::make_unique<ProcessJob>();
    data_->process.setCreateProcessArgumentsModifier([this](QProcess::CreateProcessArguments* args) { data_->job->prepare(args); });
#else
    data_->process.setUnixProcessParameters(QProcess::UnixProcessFlag::CreateNewSession);
#endif
    data_->cancelled = false; data_->group = 0; data_->active = verb; data_->decoder.resetState();
    auto& current = data_->drafts[verb]; current.lastCommand = built.displayText(); current.output.clear(); current.summaryKey = RunningKey; current.summaryArguments.clear(); current.information = false; current.networkWarning = false; current.logFailed = false;
    data_->log = std::make_unique<ConsoleLogSession>(data_->directory, data_->logging); current.logFailed = !data_->log->begin(verb, built.displayText());
    data_->process.setProgram(built.executable); data_->process.setArguments(built.arguments); data_->process.start();
    } catch (...) {
#ifdef Q_OS_WIN
        data_->job.reset();
#endif
        if (data_->process.state() != QProcess::NotRunning) { data_->process.kill(); data_->process.waitForFinished(InfiniteWait); }
        data_->active.clear(); data_->log.reset(); data_->drafts[verb].summaryKey = StartFailedKey;
        throw std::runtime_error(StartFailedKey);
    }
    publish();
}
QString GreaseweazleTools::activeOperation() const { return data_->active; }
void GreaseweazleTools::abortAndWait() {
    if(data_->active.isEmpty())return;
    data_->cancelled=true;data_->stopTimer.stop();
#ifdef Q_OS_WIN
    data_->job.reset();
#else
    if(data_->group>0)::kill(-data_->group,SIGKILL);
#endif
    if(data_->process.state()!=QProcess::NotRunning){data_->process.kill();data_->process.waitForFinished(InfiniteWait);}
    if(!data_->active.isEmpty())finish(0,false);
}
void GreaseweazleTools::stop() {
    if (data_->active.isEmpty() || data_->cancelled) return; data_->cancelled = true;
#ifdef Q_OS_WIN
    data_->process.terminate();
#else
    if (data_->group > 0) ::kill(-data_->group, SIGTERM);
#endif
    data_->stopTimer.start(StopGraceMs);
}
void GreaseweazleTools::output() {
    if (data_->active.isEmpty()) return;
    auto& current = data_->drafts[data_->active]; const QString text = data_->decoder(data_->process.readAllStandardOutput());
    current.output += text; if (current.output.size() > OutputLimit) current.output = current.output.right(OutputLimit);
    if (data_->log && !data_->log->appendText(text)) current.logFailed = true; publish();
}
void GreaseweazleTools::finish(int code, bool normal, bool startFailed) {
    if (data_->active.isEmpty() || data_->destroying) return;
    output(); data_->stopTimer.stop(); auto& current = data_->drafts[data_->active];
    current.summaryArguments.clear();
    current.summaryKey = startFailed ? StartFailedKey : data_->cancelled ? CancelledKey : normal && code == 0 ? SucceededKey : ExitCodeKey;
    if (current.summaryKey == ExitCodeKey) current.summaryArguments << QString::number(code);
    if (data_->active == Info && !startFailed && !data_->cancelled) {
        const auto value = [&](const char* label) { for (const auto& line : current.output.split(QRegularExpression(HardwareConstants::LineRegex))) { const auto colon = line.indexOf(Colon); if (colon >= 0 && line.left(colon).trimmed().compare(label, Qt::CaseInsensitive) == 0) return line.mid(colon + 1).trimmed(); } return QString(); };
        current.model = value(HardwareConstants::ModelLabel); current.firmware = value(HardwareConstants::FirmwareLabel); current.port = value(HardwareConstants::PortLabel);
        if (current.port.isEmpty()) for (const auto& line : current.output.split(QRegularExpression(HardwareConstants::LineRegex))) if (QRegularExpression(PortPattern, QRegularExpression::CaseInsensitiveOption).match(line.trimmed()).hasMatch()) { current.port = line.trimmed(); break; }
        current.networkWarning = current.output.contains(HardwareConstants::GwInfoNetworkWarning, Qt::CaseInsensitive) && (current.output.contains(HardwareConstants::GwInfoError, Qt::CaseInsensitive) || current.output.contains(HardwareConstants::GwInfoFailed, Qt::CaseInsensitive));
        current.information = !current.model.isEmpty() && ((normal && code == 0) || current.networkWarning);
    }
    if (data_->log) { auto summary = data_->translate ? data_->translate(current.summaryKey) : current.summaryKey; for (int i = 0; i < current.summaryArguments.size(); ++i) summary.replace(QString(Argument).replace(QString::number(0), QString::number(i)), current.summaryArguments[i]); if (!data_->log->appendLine(summary)) current.logFailed = true; }
    if (startFailed) writeDiagnostic(data_->directory, DiagnosticSeverity::Error, data_->process.errorString(), StartFailedKey, {}, data_->process.program());
#ifdef Q_OS_WIN
    data_->job.reset();
#else
    if (data_->group > 0) ::kill(-data_->group, SIGKILL);
#endif
    data_->group = 0; data_->log.reset(); data_->active.clear(); publish();
}
GreaseweazleTools::Subscription GreaseweazleTools::subscribe(std::function<void()> observer) { auto result = std::make_shared<const std::function<void()>>(std::move(observer)); data_->observers.push_back(result); return result; }
void GreaseweazleTools::publish() { const auto observers = data_->observers; std::erase_if(data_->observers, [](const auto& observer) { return observer.expired(); }); for (const auto& observer : observers) if (const auto live = observer.lock()) (*live)(); }
}
