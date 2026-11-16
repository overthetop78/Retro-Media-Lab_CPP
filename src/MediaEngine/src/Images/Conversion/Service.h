#pragma once
#include "Representation.h"
#include "../WritingService.h"
#include <RetroMediaLab/MediaEngine/MediaConversion.h>
namespace RetroMediaLab::MediaEngine {
class MediaConversionService final {
public:
 RETROMEDIALAB_MEDIAENGINE_API MediaConversionService(RepresentationConverterRegistry,WriterRegistry,MediaImageWritingService);
 RETROMEDIALAB_MEDIAENGINE_API std::vector<MediaConversionDestination> availableDestinations(const MediaDocument&)const;
 RETROMEDIALAB_MEDIAENGINE_API std::future<MediaConversionResult> convertAsync(MediaConversionRequest,std::stop_token={})const;
private:
 RepresentationConverterRegistry converters_;WriterRegistry writers_;MediaImageWritingService writing_;
};
}
