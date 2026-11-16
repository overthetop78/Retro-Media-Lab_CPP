#include "Fat12.h"
#include "../MediaDocument.h"
#include "../../Models/Sectors/Representation.h"
#include "../../Sources/DriveOperation.h"
#include "../../Composition/Runtime.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Fat12Conversion.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine {
std::future<void> reinterpretFat12Async(std::filesystem::path source,std::filesystem::path output,std::string target,std::stop_token token){
 return std::async(std::launch::async,[services=RuntimeDetails::defaultServices(),source=std::move(source),output=std::move(output),target=std::move(target),token]{
  checkCancellation(token);if(source.empty()||output.empty()||Primitives::isBlankIdentifier(target))throw MediaError(DiagnosticCode::ConversionRequest);
  const auto explored=services->disks.explore(source,{},{},token);
  DriveOperationScope activity(explored.document?explored.document->mediaSource()->options().drive.get():nullptr);
  Fat12Reinterpretation::convert(explored.image,output,target,explored.isHybrid(),token);activity.finish();
 });
}
std::future<void> reinterpretFat12Async(std::shared_ptr<const MediaDocument> source,std::filesystem::path output,std::string target,bool hybrid,std::stop_token token){
 return std::async(std::launch::async,[source=std::move(source),output=std::move(output),target=std::move(target),hybrid,token]{
  checkCancellation(token);if(!source)throw MediaError(DiagnosticCode::ConversionRequest);
  const auto sectors=dynamic_cast<const SectorRepresentation*>(&source->representation());if(!sectors)throw MediaError(DiagnosticCode::Fat12InvalidBpb,{source->formatId()});
  DriveOperationScope activity(source->mediaSource()->options().drive.get());
  Fat12Reinterpretation::convert(sectors->imageOwner(),output,target,hybrid,token);activity.finish();
 });
}
}
