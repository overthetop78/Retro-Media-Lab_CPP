#include "Inspection.h"
#include "../../Images/ReadRejection.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::ScpInspection {
Inspector::Inspector(MediaFileSystems::SectorRegistry fileSystems,SectorInterpretations interpretations):fileSystems_(std::move(fileSystems)),interpretations_(std::move(interpretations)){}
std::vector<MatchOwner> Inspector::inspectFileSystems(Image image,std::stop_token token)const{
 checkCancellation(token);if(!image)throw MediaError(DiagnosticCode::ReaderNullDocument);std::vector<MatchOwner> matches;
 auto report=fileSystems_.readCandidates(*image,image->geometry().formatId,token);
 for(auto& match:report.matches){
  checkCancellation(token);auto recognizedImage=image;auto normalized=interpretations_.normalize(image,match.readerId,token);
  if(normalized!=image)if(auto selected=fileSystems_.tryRead(*normalized,match.readerId,token)){recognizedImage=std::move(normalized);match=std::move(*selected);}
  const auto recognized=std::make_shared<const Match>(Match{recognizedImage,std::move(match)});matches.push_back(recognized);
  for(auto& interpretation:interpretations_.additional(recognizedImage,token))if(auto selected=fileSystems_.tryRead(*interpretation,interpretation->geometry().formatId,token))matches.push_back(std::make_shared<const Match>(Match{std::move(interpretation),std::move(*selected)}));
 }
 return matches;
}
Inspection Inspector::inspect(const ScpReconstruction::Candidate& candidate,const std::filesystem::path& path,const ScpReconstruction::Progress& progress,std::stop_token token)const{
 checkCancellation(token);
 try{auto image=candidate.readPath(path,{},progress,token);auto matches=inspectFileSystems(image,token);return {candidate.id,std::move(image),std::move(matches),{}};}
 catch(...){auto failure=std::current_exception();if(!isImageReadRejection(failure))std::rethrow_exception(failure);return {candidate.id,{},{},std::move(failure)};}
}
}
