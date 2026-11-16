#pragma once
#include <RetroMediaLab/MediaEngine/DriveMechanics.h>
namespace RetroMediaLab::MediaEngine {
class DriveAccessScope final {
public:
    DriveAccessScope(DriveMechanics* drive,DrivePosition position,bool writing):drive_(drive){
        if(drive_){if(writing)drive_->beginWrite(position);else drive_->beginRead(position);}
    }
    ~DriveAccessScope()noexcept{if(drive_)try{drive_->endAccess();}catch(...) {}}
    void finish(){if(drive_){auto* drive=drive_;drive_=nullptr;drive->endAccess();}}
    DriveAccessScope(const DriveAccessScope&)=delete;
    DriveAccessScope& operator=(const DriveAccessScope&)=delete;
private:DriveMechanics* drive_;
};
}
