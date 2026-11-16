#pragma once
#include "../ReadingService.h"
#include "../WritingService.h"
#include <RetroMediaLab/MediaEngine/MediaConversion.h>
namespace RetroMediaLab::MediaEngine {
class OpticalImageConversionService final {
public:
 RETROMEDIALAB_MEDIAENGINE_API OpticalImageConversionService(MediaImageReadingService,WriterRegistry,MediaImageWritingService);
 RETROMEDIALAB_MEDIAENGINE_API std::vector<MediaConversionDestination> availableDestinations(const MediaDocument&)const;
 RETROMEDIALAB_MEDIAENGINE_API std::future<std::vector<std::filesystem::path>> convertAsync(std::filesystem::path,std::filesystem::path,std::string,MediaConversionProgressCallback={},std::stop_token={})const;
private:
 MediaImageReadingService reading_;WriterRegistry writers_;MediaImageWritingService writing_;
};
}
