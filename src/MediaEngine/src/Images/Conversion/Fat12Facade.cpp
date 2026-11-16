#include "Fat12.h"
#include "../MediaDocument.h"
#include "../../Models/Sectors/Representation.h"
#include "../../Sources/DriveOperation.h"
#include "../../Composition/Runtime.h"
#include "../../Models/Flux/Representation.h"
#include "../../Formats/Scp/ProtectedImage.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Fat12Conversion.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace {
bool addIdentity(std::vector<std::string>& values,std::string_view value){if(std::any_of(values.begin(),values.end(),[&](const auto& old){return Primitives::ordinalIdentifierEqualsIgnoreCase(old,value);}))return false;values.emplace_back(value);return true;}
}
std::future<void> reinterpretFat12Async(std::filesystem::path source,std::filesystem::path output,std::string target,std::stop_token token){
 return std::async(std::launch::async,[services=RuntimeDetails::defaultServices(),source=std::move(source),output=std::move(output),target=std::move(target),token]{
  checkCancellation(token);if(source.empty()||output.empty()||Primitives::isBlankIdentifier(target))throw MediaError(DiagnosticCode::ConversionRequest);
  SourceOptions options;const auto utf8=source.u8string();options.primaryPath.assign(utf8.begin(),utf8.end());
  const auto document=services->reading.readingService().readAsync(std::make_shared<MediaSource>(std::move(options)),token).get();
  DriveOperationScope activity(document->mediaSource()->options().drive.get());SectorImageOwner image;std::vector<std::string> formats,readers;
  if(const auto flux=dynamic_cast<const FluxRepresentation*>(&document->representation());flux&&Primitives::asciiIdentifierEqualsIgnoreCase(document->formatId(),Scp::Constants::FormatId)){
   const auto explored=services->scpInspection.explore(source,std::make_shared<const Scp::Image>(Scp::fromProtected(flux->image(),document->metadata(),token)),{},token);
   image=!explored.detected.empty()?explored.detected.front()->image:explored.ranking.bestDecoded;
   for(const auto& match:explored.detected){addIdentity(formats,match->image->geometry().formatId);addIdentity(readers,match->match.readerId);}
   for(const auto& decoded:explored.credibleImages)addIdentity(formats,decoded->geometry().formatId);
  }else{
   if(const auto sectors=dynamic_cast<const SectorRepresentation*>(&document->representation()))image=sectors->imageOwner();
   else if(const auto otherFlux=dynamic_cast<const FluxRepresentation*>(&document->representation()))image=services->sectors.reader().read(otherFlux->image(),{},{},token);
   if(image){
    std::vector<SectorImageOwner> candidates{image};auto additional=services->interpretations.additional(image,token);candidates.insert(candidates.end(),additional.begin(),additional.end());
    auto detected=services->fileSystems.readDistinctCandidates(candidates,token);addIdentity(formats,image->geometry().formatId);
    for(const auto& match:detected){addIdentity(formats,match.image->geometry().formatId);addIdentity(readers,match.match.readerId);}
    if(!detected.empty())image=detected.front().image;
   }
  }
  if(!image)throw MediaError(DiagnosticCode::Fat12InvalidBpb,{document->formatId()});
  Fat12Reinterpretation::convert(std::move(image),output,target,formats.size()>1||readers.size()>1,token);activity.finish();
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
