#include "OpticalProbe.h"
#include <HardwareDiscoveryConstants.h>
#include <QThread>
namespace RetroMediaLab {
OpticalProbe::OpticalProbe(QObject* parent):QObject(parent){}
OpticalProbe::~OpticalProbe(){if(worker_)worker_->wait();}
void OpticalProbe::query(const QString& id,const QString& address,std::function<void(Infrastructure::OpticalCapabilities)> done,std::function<void(const char*)> failed){
    if(active_)return;
    if(worker_)worker_->wait();
    active_=true;
    worker_.reset(QThread::create([this,id,address,done=std::move(done),failed=std::move(failed)]{
        try{
            auto current=address;
            for(const auto& device:Infrastructure::discoverHardware(HardwareDiscoveryConstants::Optical))if(device.id==id){current=device.address;break;}
            const auto result=Infrastructure::queryOpticalCapabilities(current);
            QMetaObject::invokeMethod(this,[this,result,done]{active_=false;done(result);},Qt::QueuedConnection);
        }catch(...){QMetaObject::invokeMethod(this,[this,failed]{active_=false;failed(HardwareDiscoveryConstants::OpticalQueryFailed);},Qt::QueuedConnection);}
    }));
    worker_->start();
}
}
