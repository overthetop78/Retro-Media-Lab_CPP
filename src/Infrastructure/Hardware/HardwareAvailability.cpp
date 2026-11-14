#include "HardwareAvailability.h"
#include "GreaseweazleDiscovery.h"
#include "Logging/ConsoleLogSession.h"
#include <HardwareDiscoveryConstants.h>
#include <QFileInfo>
#include <QProcess>
#include <QSet>
#include <QThread>
#ifdef Q_OS_WIN
#include <windows.h>
#endif
namespace RetroMediaLab {
using namespace HardwareConstants;
namespace {
struct ProcessScope {
    QProcess process;
    ~ProcessScope(){if(process.state()!=QProcess::NotRunning){process.kill();process.waitForFinished(InfiniteWait);}}
};
QVector<Infrastructure::HostDevice> greaseweazle(const HardwareSettings& settings,const QString& directory,const LoggingSettings& logging,QStringList& errors){
    if(!QFileInfo(settings.gwPath).isFile()){errors.push_back(ToolMissingKey);return {};}
    auto hosts=Infrastructure::discoverHardware(Floppy);QVector<Infrastructure::HostDevice> confirmed;
    for(auto host:hosts){
        ProcessScope scoped;auto& process=scoped.process;
        process.setProcessChannelMode(QProcess::MergedChannels);
#ifdef Q_OS_WIN
        process.setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments* args){args->flags|=CREATE_NO_WINDOW;});
#endif
        process.setProgram(settings.gwPath);process.setArguments({Info,DeviceArgument,host.address});
        ConsoleLogSession session(directory,[logging]{return logging;});
        session.begin(Info,settings.gwPath+Space+Info+Space+DeviceArgument+Space+host.address);
        process.start();bool completed=process.waitForStarted(TimeoutMs)&&process.waitForFinished(TimeoutMs);
        if(!completed&&process.state()!=QProcess::NotRunning){process.kill();process.waitForFinished(InfiniteWait);}
        const auto output=QString::fromUtf8(process.readAllStandardOutput());session.appendText(output);
        if(GreaseweazleDiscovery::confirm(host,output,completed&&process.exitStatus()==QProcess::NormalExit&&process.exitCode()==0))confirmed.push_back(std::move(host));
        else errors.push_back(GwFailedKey);
    }
    return confirmed;
}
}
void HardwareAvailabilityResult::applyTo(HardwareSettings& settings) const{
    for(auto it=groups.cbegin();it!=groups.cend();++it){
        if(it.key()==Greaseweazle||it.key()==KryoFlux)settings.applyFloppyDiscovery(it.key(),it.value());
        else settings.applyMediaDiscovery(it.key(),it.value(),it.key()==Disks?controllers:QVector<Infrastructure::DiskController>{});
    }
}
HardwareAvailabilityResult HardwareAvailability::collect(const HardwareSettings& settings,const QString& directory,const LoggingSettings& logging){
    HardwareAvailabilityResult result;QSet<QString> requested;
    for(const auto& device:settings.devices)requested.insert(device.category==Floppy?device.backend:device.category);
    for(const auto& group:requested){
        result.groups.insert(group,{});
        try{
            if(group==Greaseweazle)result.groups[group]=greaseweazle(settings,directory,logging,result.errors);
            else {
                result.groups[group]=Infrastructure::discoverHardware(group);
                if(group==Disks)result.controllers=Infrastructure::diskControllers();
            }
        }catch(const std::exception& error){result.errors.push_back(QString::fromUtf8(error.what()));}
        catch(...){result.errors.push_back(HardwareDiscoveryConstants::Failure);}
    }
    result.errors.removeDuplicates();return result;
}
HardwareAvailability::HardwareAvailability(QObject* parent):QObject(parent){}
HardwareAvailability::~HardwareAvailability(){if(worker_)worker_->wait();}
void HardwareAvailability::refresh(HardwareSettings settings,QString directory,LoggingSettings logging,std::function<void(HardwareAvailabilityResult)> done){
    if(active_)return;
    if(worker_)worker_->wait();active_=true;
    worker_.reset(QThread::create([this,settings=std::move(settings),directory=std::move(directory),logging=std::move(logging),done=std::move(done)]{
        auto result=collect(settings,directory,logging);
        QMetaObject::invokeMethod(this,[this,result=std::move(result),done]{active_=false;done(result);},Qt::QueuedConnection);
    }));worker_->start();
}
}
