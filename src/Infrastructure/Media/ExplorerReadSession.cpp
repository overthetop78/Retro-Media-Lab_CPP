#include "ExplorerReadSession.h"
#include <ErrorKeys.h>
#include "Maintenance/GwToolsConstants.h"
#include <QTemporaryDir>
#include <QDir>
#include <stdexcept>
namespace RetroMediaLab::Infrastructure {
namespace C=MediaCommandConstants;
ExplorerReadSession::ExplorerReadSession(std::shared_ptr<ExplorerSession> opening,std::shared_ptr<GreaseweazleTools> tools,std::shared_ptr<OperationProfiles> profiles):opening_(std::move(opening)),capture_(std::make_shared<CaptureSession>(CaptureMode::ExplorerRead,std::move(tools),std::move(profiles))){
    captureSubscription_=capture_->subscribe([this]{update();});openingSubscription_=opening_->subscribe([this]{if(openingCapture_ && !opening_->busy())openingCapture_=false;});
}
ExplorerReadSession::~ExplorerReadSession(){captureSubscription_.reset();openingSubscription_.reset();if(openingCapture_)opening_->cancel();capture_.reset();directory_.reset();}
bool ExplorerReadSession::busy()const noexcept{return capturing_ || openingCapture_;}
void ExplorerReadSession::start(const HardwareSettings& hardware,const EngineSettings& engines,const QString& applicationDirectory,const QString& deviceId){
    if(!canStart())throw std::invalid_argument(GwToolsConstants::BusyKey);
    directory_=std::make_shared<QTemporaryDir>(QDir(QDir::tempPath()).filePath(QString::fromLatin1(C::ExplorerTemporaryPattern)));if(!directory_->isValid()){directory_.reset();throw std::invalid_argument(ErrorKeys::InvalidImageDirectory);}
    auto& draft=capture_->draft;draft.directory=directory_->path();draft.fileName=QString::fromLatin1(C::ExplorerCaptureName);draft.deviceId=deviceId;draft.raw=true;draft.autoNumber=false;draft.enabled.clear();draft.expert.clear();
    capturing_=true;
    try{capture_->start(hardware,engines,applicationDirectory);}catch(...){capturing_=false;directory_.reset();throw;}
}
void ExplorerReadSession::stop(){if(capturing_)capture_->stop();else if(openingCapture_)opening_->cancel();}
void ExplorerReadSession::update(){
    if(!capturing_ || capture_->busy())return;capturing_=false;
    if(!capture_->completedPath().isEmpty()){
        openingCapture_=true;opening_->open(capture_->completedPath(),{},directory_);openingCapture_=opening_->busy();
    }
    directory_.reset();
}
}
