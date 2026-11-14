#include "GreaseweazleDiscovery.h"
#include "HardwareConstants.h"
#include "Logging/ConsoleLogSession.h"
#include <stdexcept>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QTimer>
#ifdef Q_OS_WIN
#include <windows.h>
#endif
namespace RetroMediaLab {
using namespace HardwareConstants;
GreaseweazleDiscovery::GreaseweazleDiscovery(QString directory,std::function<LoggingSettings()> logging,QObject* parent):QObject(parent),logsDirectory_(std::move(directory)),logging_(std::move(logging)),process_(std::make_unique<QProcess>(this)),timeout_(std::make_unique<QTimer>(this)){
    process_->setProcessChannelMode(QProcess::MergedChannels);timeout_->setSingleShot(true);
#ifdef Q_OS_WIN
    process_->setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments* arguments){arguments->flags|=CREATE_NO_WINDOW;});
#endif
    connect(timeout_.get(),&QTimer::timeout,this,[this]{process_->kill();});
    connect(process_.get(),&QProcess::finished,this,[this](int code,QProcess::ExitStatus status){finish(code==0&&status==QProcess::NormalExit);});
    connect(process_.get(),&QProcess::errorOccurred,this,[this](QProcess::ProcessError error){if(error==QProcess::FailedToStart)finish(false);});
}
GreaseweazleDiscovery::~GreaseweazleDiscovery(){
    active_=false;timeout_->stop();process_->disconnect(this);
    if(process_->state()!=QProcess::NotRunning){process_->kill();process_->waitForFinished(InfiniteWait);}
}
void GreaseweazleDiscovery::scan(const QString& executable,std::function<void(QVector<Infrastructure::HostDevice>)> done,std::function<void(const char*)> failed){
    if(active_)return;
    if(!QFileInfo(executable).isFile()){failed(ToolMissingKey);return;}
    try{pending_=Infrastructure::discoverHardware(Floppy);}catch(const std::exception& error){failed(error.what());return;}
    process_->setProgram(executable);done_=std::move(done);failed_=std::move(failed);found_.clear();active_=true;anyFailure_=false;next();
}
void GreaseweazleDiscovery::next(){
    if(!active_)return;
    if(pending_.isEmpty()){
        active_=false;auto done=std::move(done_);auto failed=std::move(failed_);
        done(found_);if(anyFailure_)failed(GwFailedKey);return;
    }
    current_=pending_.takeFirst();process_->setArguments({Info,DeviceArgument,current_.address});
    session_=std::make_unique<ConsoleLogSession>(logsDirectory_,logging_);
    session_->begin(Info,process_->program()+Space+Info+Space+DeviceArgument+Space+current_.address);
    process_->start();timeout_->start(TimeoutMs);
}
void GreaseweazleDiscovery::finish(bool successful){
    if(!active_)return;timeout_->stop();const auto output=QString::fromUtf8(process_->readAllStandardOutput());
    if(session_)session_->appendText(output);
    if(confirm(current_,output,successful))found_.push_back(current_);else anyFailure_=true;
    QTimer::singleShot(0,this,[this]{next();});
}
bool GreaseweazleDiscovery::confirm(Infrastructure::HostDevice& host,const QString& output,bool successful){
    const auto value=[&](const char* key){for(const auto& line:output.split(QRegularExpression(LineRegex))){
        const auto colon=line.indexOf(Colon);if(colon>=0&&line.left(colon).trimmed().compare(key,Qt::CaseInsensitive)==0)return line.mid(colon+1).trimmed();}return QString();};
    const auto model=value(ModelLabel),serial=value(SerialLabel);
    const bool networkWarning=output.contains(GwInfoNetworkWarning,Qt::CaseInsensitive)
        &&(output.contains(GwInfoError,Qt::CaseInsensitive)||output.contains(GwInfoFailed,Qt::CaseInsensitive));
    if(!successful&&!(networkWarning&&!model.isEmpty()&&value(PortLabel).compare(host.address,Qt::CaseInsensitive)==0
        &&(!value(FirmwareLabel).isEmpty()||!serial.isEmpty())))return false;
    if(!serial.isEmpty())host.id=serial;if(!model.isEmpty())host.name=model;return true;
}
}
