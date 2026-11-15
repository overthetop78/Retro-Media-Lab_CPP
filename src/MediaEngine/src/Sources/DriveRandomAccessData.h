#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaEngine/DriveMechanics.h>
#include <RetroMediaLab/MediaFileSystems/Sources.h>
#include <functional>
namespace RetroMediaLab::MediaEngine {
class DriveRandomAccessData final:public MediaFileSystems::RandomAccessData {
public:
    RETROMEDIALAB_MEDIAENGINE_API DriveRandomAccessData(std::shared_ptr<const MediaFileSystems::RandomAccessData>,std::shared_ptr<DriveMechanics>,std::function<DrivePosition(std::int64_t)> position,std::function<std::int64_t(std::int64_t)> contiguousLength,bool writing);
    const std::shared_ptr<DriveMechanics>& drive()const noexcept{return drive_;}
    bool writing()const noexcept{return writing_;}
    RETROMEDIALAB_MEDIAENGINE_API std::uint64_t length()const noexcept override;
    RETROMEDIALAB_MEDIAENGINE_API void readExactly(std::uint64_t,MediaFileSystems::MutableBytes,std::stop_token={})const override;
private:
    std::shared_ptr<const MediaFileSystems::RandomAccessData> source_;
    std::shared_ptr<DriveMechanics> drive_;
    std::function<DrivePosition(std::int64_t)> position_;
    std::function<std::int64_t(std::int64_t)> contiguous_;
    bool writing_;
};
}
