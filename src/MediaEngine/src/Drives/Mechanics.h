#pragma once
#include <RetroMediaLab/MediaEngine/DriveMechanics.h>
#include "../Audio/Mechanical/RecordedRenderer.h"
#include <functional>
#include <mutex>
#include <deque>
namespace RetroMediaLab::MediaEngine {
class RecordedDrive final:public DriveMechanics {
public:
    using Notification=std::function<void(DriveMechanics&)>;
    RETROMEDIALAB_MEDIAENGINE_API RecordedDrive(std::string device,std::shared_ptr<MechanicalAudio::RecordedRenderer> bank={},Notification={},std::optional<std::string> profile={},double volume=MechanicalAudio::Constants::DriveDefaultVolume,bool enabled=true);
    RETROMEDIALAB_MEDIAENGINE_API DriveState state()const override;
    RETROMEDIALAB_MEDIAENGINE_API bool hasPendingAudio()const override;
    RETROMEDIALAB_MEDIAENGINE_API bool isInUse()const override;
    RETROMEDIALAB_MEDIAENGINE_API void beginOperation()override;
    RETROMEDIALAB_MEDIAENGINE_API void endOperation()override;
    RETROMEDIALAB_MEDIAENGINE_API void configure(std::optional<std::string>,double,bool)override;
    RETROMEDIALAB_MEDIAENGINE_API void setTapeOperation(std::string_view)override;
    RETROMEDIALAB_MEDIAENGINE_API void insert()override;
    RETROMEDIALAB_MEDIAENGINE_API void eject()override;
    RETROMEDIALAB_MEDIAENGINE_API void setMotor(bool)override;
    RETROMEDIALAB_MEDIAENGINE_API void setActivity(bool,bool)override;
    RETROMEDIALAB_MEDIAENGINE_API void seek(DrivePosition)override;
    RETROMEDIALAB_MEDIAENGINE_API void advance(DrivePosition)override;
    RETROMEDIALAB_MEDIAENGINE_API void beginRead(DrivePosition)override;
    RETROMEDIALAB_MEDIAENGINE_API void beginWrite(DrivePosition)override;
    RETROMEDIALAB_MEDIAENGINE_API void endAccess()override;
    RETROMEDIALAB_MEDIAENGINE_API std::vector<std::int16_t> renderAudio(std::int32_t,std::int32_t)override;
    RETROMEDIALAB_MEDIAENGINE_API void reset()override;
private:
    struct Sound {std::string operation;int distance=1,direction=1,cylinder=-1;};
    struct Voice {Sound sound;std::int64_t offset=0;};
    mutable std::mutex mutex_;
    std::string device_;std::shared_ptr<MechanicalAudio::RecordedRenderer> bank_;Notification notification_;
    DriveState state_;std::optional<std::string> profile_;double volume_=0;
    bool enabled_=false,audibleMotor_=false,stopAfterAccess_=false;
    std::int64_t motorOffset_=0,eventOffset_=0,tapeOffset_=0;
    std::uint64_t activeOperations_=0,activeAccesses_=0;
    std::string tapeOperation_=std::string(MechanicalAudio::Constants::None);
    std::deque<Sound> events_;std::vector<Voice> voices_;
    void motorCore(bool);void seekCore(DrivePosition);void beginAccess(DrivePosition,bool);void enqueue(Sound);void clearAudio();void notify();
    std::string_view selectedProfile()const;
    std::int64_t frames(const Sound&,int);
    std::vector<std::int16_t> render(const Sound&,int,int,std::int64_t);
};
}
