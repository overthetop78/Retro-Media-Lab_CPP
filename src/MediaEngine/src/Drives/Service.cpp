#include <RetroMediaLab/MediaEngine/DriveService.h>
#include "Mechanics.h"
namespace RetroMediaLab::MediaEngine {
struct MediaDriveService::State {std::mutex mutex;Notification notification;};
MediaDriveService::MediaDriveService():state_(std::make_shared<State>()){}
void MediaDriveService::setAudioAvailable(Notification notification)const{std::lock_guard lock(state_->mutex);state_->notification=std::move(notification);}
std::shared_ptr<DriveMechanics> MediaDriveService::create(MediaKind kind)const{
    namespace C=MechanicalAudio::Constants;
    std::string_view device;switch(kind){case MediaKind::Floppy:device=C::Floppy;break;case MediaKind::HardDisk:device=C::HardDisk;break;case MediaKind::Optical:device=C::Optical;break;default:return {};}
    const auto weak=std::make_shared<std::weak_ptr<DriveMechanics>>();
    auto drive=std::make_shared<RecordedDrive>(std::string(device),nullptr,[state=state_,weak](DriveMechanics&){
        Notification notification;{std::lock_guard lock(state->mutex);notification=state->notification;}
        if(notification)if(auto source=weak->lock())notification(std::move(source));
    });
    *weak=drive;return drive;
}
}
