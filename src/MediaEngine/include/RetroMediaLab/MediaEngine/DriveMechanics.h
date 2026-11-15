#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
namespace RetroMediaLab::MediaEngine {
struct DrivePosition {std::int32_t cylinder=0,head=0,sector=-1;std::int64_t logicalBlock=-1;bool operator==(const DrivePosition&)const=default;};
struct DriveState {bool motorRunning=false,mediaInserted=false,reading=false,writing=false;DrivePosition position;bool operator==(const DriveState&)const=default;};
// The caller owns the clock and audio output. Bindings share lifetime, never reset the drive.
class DriveMechanics {
public:
    virtual ~DriveMechanics()=default;
    virtual DriveState state()const=0;
    virtual bool hasPendingAudio()const=0;
    virtual bool isInUse()const=0;
    virtual void beginOperation()=0;
    virtual void endOperation()=0;
    virtual void configure(std::optional<std::string> profile,double volumePercent,bool soundEnabled)=0;
    virtual void setTapeOperation(std::string_view)=0;
    virtual void insert()=0;
    virtual void eject()=0;
    virtual void setMotor(bool)=0;
    virtual void setActivity(bool reading,bool writing)=0;
    virtual void seek(DrivePosition)=0;
    virtual void advance(DrivePosition)=0;
    virtual void beginRead(DrivePosition)=0;
    virtual void beginWrite(DrivePosition)=0;
    virtual void endAccess()=0;
    virtual std::vector<std::int16_t> renderAudio(std::int32_t sampleRate,std::int32_t sampleCount)=0;
    virtual void reset()=0;
};
}
