#include <RetroMediaLab/MediaEngine/DriveService.h>
#include "Mechanics.h"
namespace RetroMediaLab::MediaEngine {
struct MediaDriveService::State {std::mutex mutex;Notification notification;};
MediaDriveService::MediaDriveService():state_(std::make_shared<State>()){}
void MediaDriveService::setAudioAvailable(Notification notification)const{std::lock_guard lock(state_->mutex);state_->notification=std::move(notification);}
std::shared_ptr<DriveMechanics> MediaDriveService::create(MediaKind kind)const{
    namespace C=MechanicalAudio::Constants;
    std::string_view device;switch(kind){case MediaKind::Floppy:device=C::Floppy;break;case MediaKind::HardDisk:device=C::HardDisk;break;case MediaKind::Optical:device=C::Optical;break;default:return {};}
    return std::make_shared<RecordedDrive>(std::string(device),nullptr,[state=state_](DriveMechanics& drive){
        Notification notification;{std::lock_guard lock(state->mutex);notification=state->notification;}if(notification)notification(drive);
    });
}
}
