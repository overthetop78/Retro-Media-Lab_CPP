#pragma once
#include <RetroMediaLab/MediaEngine/DriveMechanics.h>
namespace RetroMediaLab::MediaEngine {
class DriveOperationScope final {
public:
    explicit DriveOperationScope(DriveMechanics* drive):drive_(drive){if(drive_)drive_->beginOperation();}
    ~DriveOperationScope()noexcept{if(drive_)try{drive_->endOperation();}catch(...){}}
    void finish(){if(drive_){auto* drive=drive_;drive_=nullptr;drive->endOperation();}}
    DriveOperationScope(const DriveOperationScope&)=delete;
    DriveOperationScope& operator=(const DriveOperationScope&)=delete;
private:DriveMechanics* drive_;
};
}
