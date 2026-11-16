#include "Inspection.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaFileSystems/Interpretation.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::ScpInspection {
Service::Service(std::shared_ptr<const Scp::CaptureReader> captures,ScpReconstruction::Candidates candidates,Decoding::DecodeOperation decode,Inspector inspector)
 :captures_(std::move(captures)),candidates_(std::move(candidates)),decode_(std::move(decode)),inspector_(std::move(inspector)){if(!captures_||!decode_)throw MediaError(DiagnosticCode::ConversionRequest);}
Result Service::explore(const std::filesystem::path& path,Scp::SharedImage capture,const Progress& progress,std::stop_token token)const{
 checkCancellation(token);if(!capture)throw MediaError(DiagnosticCode::ReaderNullDocument);captures_->remember(path,std::move(capture));return explore(path,progress,token);
}
Result Service::explore(const std::filesystem::path& path,const Progress& progress,std::stop_token token)const{
 checkCancellation(token);Result result;result.capture=captures_->readFileAsync(path,token).get();
 if(!result.capture)throw MediaError(DiagnosticCode::ReaderNullDocument);
 const auto families=probeFamilies(*result.capture,decode_,progress,token);if(families.empty())return result;
 const auto candidates=candidates_.automatic(families);std::vector<Inspection> inspections;inspections.reserve(candidates.size());
 for(std::size_t index=0;index<candidates.size();++index){
  checkCancellation(token);const auto& candidate=candidates[index];
  if(progress)progress({ProgressKind::CandidateStarted,std::string(candidate.displayName()),index,candidates.size(),{},{},{}});
  ScpReconstruction::Progress units;if(progress)units=[&](const auto& value){progress({value.kind==IsoReconstruction::CollectionProgressKind::TrackDecoded?ProgressKind::TrackDecoded:ProgressKind::RevolutionDecoded,candidate.id,std::size_t(value.completed),std::size_t(value.total),{},value.cylinder,value.head});};
  inspections.push_back(inspector_.inspect(candidate,path,units,token));
  if(progress)progress({ProgressKind::CandidateCompleted,std::string(candidate.displayName()),index+1,candidates.size(),bool(inspections.back().image),{},{}});
 }
 for(auto& inspection:inspections){
  checkCancellation(token);auto normalized=normalizeCompatibleGeometry(inspection.image,*result.capture,token);if(!normalized||normalized==inspection.image)continue;
  inspection.image=std::move(normalized);inspection.matches=inspector_.inspectFileSystems(inspection.image,token);
 }
 result.ranking=rank(std::move(inspections),*result.capture,token);if(!result.ranking.bestDecoded)return result;
 using Identity=MediaFileSystems::FileSystemInterpretationIdentity;using Policy=MediaFileSystems::FileSystemAlternativePolicy;
 const auto& primary=result.ranking.bestFileSystem;
 if(primary){
  result.detected.push_back(primary);const auto identity=Identity::create(primary->image->geometry().formatId,primary->match.volume);
  for(const auto& match:result.ranking.detected)if(Identity::create(match->image->geometry().formatId,match->match.volume)!=identity&&Policy::isCredible(match->match.volume))result.detected.push_back(match);
 }else result.detected=result.ranking.detected;
 std::vector<std::string> recognized,familiesSeen;for(const auto& match:result.detected)recognized.push_back(match->image->geometry().formatId);
 for(const auto& image:result.ranking.decodedImages){
  checkCancellation(token);const auto& format=image->geometry().formatId;
  if(std::any_of(recognized.begin(),recognized.end(),[&](const auto& id){return Primitives::ordinalIdentifierEqualsIgnoreCase(id,format);}))continue;
  const auto family=Identity::formatFamily(format);if(std::any_of(familiesSeen.begin(),familiesSeen.end(),[&](const auto& old){return Primitives::ordinalIdentifierEqualsIgnoreCase(old,family);}))continue;
  familiesSeen.push_back(family);result.credibleImages.push_back(image);
 }
 return result;
}
}
