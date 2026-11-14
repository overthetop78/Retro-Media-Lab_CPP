#include "DiskProbe.h"
#include "DiskProbeService.h"
#include <HardwareDiscoveryConstants.h>
#include <QThread>
#include <stdexcept>
namespace RetroMediaLab {
DiskProbe::DiskProbe(QObject* parent):QObject(parent){}
DiskProbe::~DiskProbe(){if(worker_)worker_->wait();}
void DiskProbe::query(const QString& controller,const QString& disk,std::function<void(Infrastructure::DiskAccessReport)> done,std::function<void(const char*)> failed){
    if(active_)return;
    if(worker_)worker_->wait();
    active_=true;
    worker_.reset(QThread::create([this,controller,disk,done=std::move(done),failed=std::move(failed)]{
        try{
            const auto result=Infrastructure::runDiskProbe(controller,disk);
            QMetaObject::invokeMethod(this,[this,result,done]{active_=false;done(result);},Qt::QueuedConnection);
        }catch(...){QMetaObject::invokeMethod(this,[this,failed]{active_=false;failed(HardwareDiscoveryConstants::DiskQueryFailed);},Qt::QueuedConnection);}
    }));
    worker_->start();
}
}
