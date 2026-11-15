#pragma once
#include "MediaSource.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/MediaKind.h>
#include <RetroMediaLab/MediaEngine/Export.h>
#include <functional>
namespace RetroMediaLab::MediaEngine {
using DriveFactory=std::function<std::shared_ptr<DriveMechanics>(MediaKind)>;
class RecognitionContext final {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit RecognitionContext(std::shared_ptr<const MediaSource>,std::int32_t pageSize=ImageConstants::PageSize,std::shared_ptr<const MediaFileSystems::RandomAccessData> data={},DriveFactory drives={});
    RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaSource> source()const;
    RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaFileSystems::RandomAccessData> dataSource()const;
    RETROMEDIALAB_MEDIAENGINE_API std::int64_t length()const noexcept;
    RETROMEDIALAB_MEDIAENGINE_API const std::string& extension()const noexcept;
    RETROMEDIALAB_MEDIAENGINE_API std::optional<std::string> requestedFormatId()const;
    RETROMEDIALAB_MEDIAENGINE_API bool ownsDrive()const noexcept;
    RETROMEDIALAB_MEDIAENGINE_API void ensureDrive(MediaKind)const;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::vector<std::uint8_t>> readHeaderAsync(std::int32_t count,std::stop_token={})const;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::vector<std::uint8_t>> readAsync(std::int64_t offset,std::int32_t count,std::stop_token={})const;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::vector<std::uint8_t>> readBytesAsync(std::stop_token={})const;
private:struct Impl;std::shared_ptr<Impl> impl_;
};
RETROMEDIALAB_MEDIAENGINE_API bool sameIdentifier(std::string_view,std::string_view)noexcept;
}
