#include "Mechanics.h"
#include "../Audio/Mechanical/RecordedBank.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <cmath>
#include <limits>
namespace RetroMediaLab::MediaEngine {
namespace C=MechanicalAudio::Constants;
namespace {
void validate(DrivePosition position){if(position.cylinder<0||position.head<0)throw MediaError(DiagnosticCode::MechanicalRequest);}
}
RecordedDrive::RecordedDrive(std::string device,std::shared_ptr<MechanicalAudio::RecordedRenderer> bank,Notification notification,std::optional<std::string> profile,double volume,bool enabled)
    :device_(std::move(device)),bank_(std::move(bank)),notification_(std::move(notification)){
    if(!MechanicalAudio::baseRecordedProfile(device_))throw MediaError(DiagnosticCode::MechanicalRequest);
    if(!bank_)bank_=MechanicalAudio::sharedRecordedBank();
    // Construction publishes no callback before the object is fully initialized.
    if(!std::isfinite(volume)||volume<0||volume>C::MaximumVolumePercent)throw MediaError(DiagnosticCode::MechanicalRequest);
    profile_=std::move(profile);volume_=volume;enabled_=enabled;
}
DriveState RecordedDrive::state()const{std::lock_guard lock(mutex_);return state_;}
bool RecordedDrive::hasPendingAudio()const{std::lock_guard lock(mutex_);return enabled_&&(tapeOperation_!=C::None||!events_.empty()||!voices_.empty()||audibleMotor_||state_.motorRunning);}
bool RecordedDrive::isInUse()const{std::lock_guard lock(mutex_);return activeOperations_||activeAccesses_||state_.reading||state_.writing;}
void RecordedDrive::notify(){if(hasPendingAudio()&&notification_)notification_(*this);}
void RecordedDrive::configure(std::optional<std::string> profile,double volume,bool enabled){
    if(!std::isfinite(volume)||volume<0||volume>C::MaximumVolumePercent)throw MediaError(DiagnosticCode::MechanicalRequest);
    {std::lock_guard lock(mutex_);profile_=std::move(profile);volume_=volume;if(!enabled_&&enabled&&state_.motorRunning){audibleMotor_=true;motorOffset_=0;}enabled_=enabled;if(!enabled)clearAudio();}notify();
}
void RecordedDrive::beginOperation(){std::lock_guard lock(mutex_);if(activeOperations_==std::numeric_limits<std::uint64_t>::max())throw MediaError(DiagnosticCode::MechanicalRequest);++activeOperations_;stopAfterAccess_=false;}
void RecordedDrive::endOperation(){{std::lock_guard lock(mutex_);if(activeOperations_)--activeOperations_;if(!activeOperations_){if(!activeAccesses_)motorCore(false);else stopAfterAccess_=true;}}notify();}
void RecordedDrive::insert(){{std::lock_guard lock(mutex_);if(state_.mediaInserted)return;state_.mediaInserted=true;enqueue({std::string(C::Insert)});}notify();}
void RecordedDrive::eject(){{std::lock_guard lock(mutex_);motorCore(false);if(state_.mediaInserted){state_.mediaInserted=false;enqueue({std::string(C::Eject)});}}notify();}
void RecordedDrive::motorCore(bool running){if(state_.motorRunning==running)return;state_.motorRunning=running;state_.reading=state_.writing=false;if(running){audibleMotor_=enabled_;motorOffset_=0;}else enqueue({std::string(C::Stop)});}
void RecordedDrive::setMotor(bool running){{std::lock_guard lock(mutex_);stopAfterAccess_=false;motorCore(running);}notify();}
void RecordedDrive::setActivity(bool reading,bool writing){std::lock_guard lock(mutex_);state_.reading=reading;state_.writing=writing;}
void RecordedDrive::seekCore(DrivePosition position){
    const auto displacement=std::int64_t(position.cylinder)-state_.position.cylinder;
    if(displacement)enqueue({std::string(C::Seek),static_cast<int>(std::abs(displacement)),(displacement>0)-(displacement<0),position.cylinder});
    else if(position.logicalBlock>=0&&state_.position.logicalBlock>=0&&position.logicalBlock!=state_.position.logicalBlock
        &&(state_.position.logicalBlock==std::numeric_limits<std::int64_t>::max()||position.logicalBlock!=state_.position.logicalBlock+1))
        enqueue({std::string(C::Seek),0,(position.logicalBlock>state_.position.logicalBlock)-(position.logicalBlock<state_.position.logicalBlock)});
    state_.position=position;
}
void RecordedDrive::seek(DrivePosition position){validate(position);{std::lock_guard lock(mutex_);seekCore(position);}notify();}
void RecordedDrive::advance(DrivePosition position){validate(position);std::lock_guard lock(mutex_);state_.position=position;}
void RecordedDrive::beginAccess(DrivePosition position,bool writing){
    validate(position);
    {
        std::lock_guard lock(mutex_);
        if(activeAccesses_==std::numeric_limits<std::uint64_t>::max())throw MediaError(DiagnosticCode::MechanicalRequest);
        const auto previous=state_;const auto previousAudible=audibleMotor_;const auto previousOffset=motorOffset_;
        try {motorCore(true);seekCore(position);}
        catch(...){state_=previous;audibleMotor_=previousAudible;motorOffset_=previousOffset;throw;}
        ++activeAccesses_;state_.reading=!writing;state_.writing=writing;
    }
    try {notify();}
    catch(...){
        // Only this begin acquired an additional access. Preserve other active users.
        {std::lock_guard lock(mutex_);if(activeAccesses_==1&&!activeOperations_)stopAfterAccess_=true;}
        try{endAccess();}catch(...){}
        throw;
    }
}
void RecordedDrive::beginRead(DrivePosition position){beginAccess(position,false);}
void RecordedDrive::beginWrite(DrivePosition position){beginAccess(position,true);}
void RecordedDrive::endAccess(){bool stopped=false;{std::lock_guard lock(mutex_);if(activeAccesses_)--activeAccesses_;if(!activeAccesses_){state_.reading=state_.writing=false;if(stopAfterAccess_){stopAfterAccess_=false;motorCore(false);stopped=true;}}}if(stopped)notify();}
void RecordedDrive::setTapeOperation(std::string_view operation){
    if(device_!=C::Tape||(operation!=C::None&&operation!=C::Insert&&operation!=C::Eject&&operation!=C::Stop&&operation!=C::Pause&&operation!=C::Play&&operation!=C::Record&&operation!=C::FastForward&&operation!=C::Rewind))throw MediaError(DiagnosticCode::MechanicalRequest);
    {std::lock_guard lock(mutex_);
    if(operation==C::Play||operation==C::Record||operation==C::FastForward||operation==C::Rewind){if(tapeOperation_==operation)return;tapeOperation_=operation;tapeOffset_=0;}
    else {if(operation!=C::Insert&&operation!=C::Pause){tapeOperation_=C::None;tapeOffset_=0;}events_.clear();eventOffset_=0;if(operation!=C::None)enqueue({std::string(operation)});}}
    notify();
}
void RecordedDrive::enqueue(Sound sound){if(!enabled_)return;if(sound.operation==C::Seek){if(voices_.size()==C::MaximumSeekVoices)voices_.erase(voices_.begin());voices_.push_back({std::move(sound),0});}else events_.push_back(std::move(sound));}
void RecordedDrive::clearAudio(){tapeOperation_=C::None;tapeOffset_=0;events_.clear();voices_.clear();audibleMotor_=false;eventOffset_=motorOffset_=0;}
void RecordedDrive::reset(){std::lock_guard lock(mutex_);state_={};activeOperations_=activeAccesses_=0;stopAfterAccess_=false;clearAudio();}
std::string_view RecordedDrive::selectedProfile()const{if(profile_&&MechanicalAudio::findRecordedProfile(*profile_))return *profile_;return *MechanicalAudio::baseRecordedProfile(device_);}
}
