#include "Fat12.h"
#include "../MediaDocument.h"
#include "../../Models/Sectors/Representation.h"
#include "../../Sources/DriveOperation.h"
#include <RetroMediaLab/MediaEngine/Fat12Conversion.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine {
std::future<void> reinterpretFat12Async(std::shared_ptr<const MediaDocument> source,std::filesystem::path output,std::string target,bool hybrid,std::stop_token token){
 return std::async(std::launch::async,[source=std::move(source),output=std::move(output),target=std::move(target),hybrid,token]{
  checkCancellation(token);if(!source)throw MediaError(DiagnosticCode::ConversionRequest);
  const auto sectors=dynamic_cast<const SectorRepresentation*>(&source->representation());if(!sectors)throw MediaError(DiagnosticCode::Fat12InvalidBpb,{source->formatId()});
  DriveOperationScope activity(source->mediaSource()->options().drive.get());
  Fat12Reinterpretation::convert(sectors->imageOwner(),output,target,hybrid,token);activity.finish();
 });
}
}
