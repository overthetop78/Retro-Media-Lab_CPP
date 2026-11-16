#pragma once
#include "Representation.h"
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include "../WritingService.h"
#include <RetroMediaLab/MediaEngine/MediaConversion.h>
namespace RetroMediaLab::MediaEngine {
class MediaConversionService final {
public:
 using SectorRead=std::function<std::future<std::shared_ptr<const MediaFileSystems::SectorImage>>() >;
 RETROMEDIALAB_MEDIAENGINE_API static std::future<std::shared_ptr<const MediaFileSystems::SectorImage>> readSectorSource(const std::filesystem::path&,const SectorRead& readScp,const SectorRead& readImage);
 RETROMEDIALAB_MEDIAENGINE_API MediaConversionService(RepresentationConverterRegistry,WriterRegistry,MediaImageWritingService);
 RETROMEDIALAB_MEDIAENGINE_API std::vector<MediaConversionDestination> availableDestinations(const MediaDocument&)const;
 RETROMEDIALAB_MEDIAENGINE_API std::future<MediaConversionResult> convertAsync(MediaConversionRequest,std::stop_token={})const;
private:
 RepresentationConverterRegistry converters_;WriterRegistry writers_;MediaImageWritingService writing_;
};
}
