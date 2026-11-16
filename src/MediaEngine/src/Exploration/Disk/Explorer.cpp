#include "Explorer.h"
#include "Constants.h"
#include "../Metadata/Metadata.h"
#include "../../Models/Sectors/SectorImages.h"
#include "../../Models/Sectors/Representation.h"
#include "../../Models/Flux/Representation.h"
#include "../../Formats/Scp/ProtectedImage.h"
#include "../../Sources/DriveOperation.h"
#include "../../Images/ReadRejection.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QFileInfo>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace {
bool same(std::string_view a,std::string_view b){return Primitives::ordinalIdentifierEqualsIgnoreCase(a,b);}
void add(std::vector<std::string>& values,std::string_view value){if(std::none_of(values.begin(),values.end(),[&](const auto& old){return same(old,value);}))values.emplace_back(value);}
std::string utf8(const std::filesystem::path& path){const auto text=path.u8string();return {text.begin(),text.end()};}
DiskExploration make(const std::filesystem::path& path,std::shared_ptr<const MediaDocument> document,SectorImageOwner image,std::vector<ScpInspection::MatchOwner> detected,std::vector<SectorImageOwner> images,Scp::SharedImage capture,std::stop_token token){
 DiskExploration result;result.sourcePath=path;result.document=std::move(document);result.image=detected.empty()?std::move(image):detected.front()->image;result.detected=std::move(detected);result.capture=std::move(capture);result.fileSystemRecognized=!result.detected.empty();result.primaryFormatId=result.image->geometry().formatId;
 std::vector<std::string> formats;for(auto& candidate:images){if(std::any_of(formats.begin(),formats.end(),[&](const auto& id){return same(id,candidate->geometry().formatId);}))continue;formats.push_back(candidate->geometry().formatId);result.decodedImages.push_back(std::move(candidate));}result.metadata=DiskMetadata::create(*result.image,result.detectedFormats(),token);return result;
}
DiskExploration unknown(const std::filesystem::path& path,std::shared_ptr<const MediaDocument> document={},Scp::SharedImage capture={},std::stop_token token={}){
 using namespace DiskExplorationConstants;const QFileInfo info(QString::fromStdU16String(path.u16string()));if(!info.isFile()||info.size()<0)throw MediaError(DiagnosticCode::AtomicOpen,{utf8(path)});
 auto image=sparseSectorImage({std::string(UnknownFormat),UnknownBlockSize,UnknownDimension,UnknownDimension,UnknownDimension},{},{false,std::uint64_t(info.size()),UnknownBlockCount});
 return make(path,std::move(document),image,{},{image},std::move(capture),token);
}
bool rejectedFlux(const std::exception_ptr& failure){
 if(isImageReadRejection(failure)||isUnsupportedImageRead(failure))return true;
 try{std::rethrow_exception(failure);}catch(const MediaError& error){return error.diagnostic().code==DiagnosticCode::ReaderAllRejected&&std::all_of(error.causes().begin(),error.causes().end(),[](const auto& cause){return isImageReadRejection(cause.exception);});}catch(...){return false;}
}
}
std::vector<std::string> DiskExploration::detectedFormats()const{std::vector<std::string> formats;for(const auto& match:detected)add(formats,match->image->geometry().formatId);for(const auto& decoded:decodedImages)add(formats,decoded->geometry().formatId);return formats;}
bool DiskExploration::isHybrid()const{if(detectedFormats().size()>1)return true;std::vector<std::string> readers;for(const auto& match:detected)add(readers,match->match.readerId);return readers.size()>1;}
std::optional<DiskExploration> DiskExploration::selectFormat(std::string_view format)const{
 if(Primitives::isBlankIdentifier(format))throw MediaError(DiagnosticCode::ConversionRequest);
 for(const auto& match:detected)if(same(match->image->geometry().formatId,format)){auto selected=*this;selected.image=match->image;selected.primaryFormatId=match->image->geometry().formatId;selected.fileSystemRecognized=true;return selected;}
 for(const auto& decoded:decodedImages)if(same(decoded->geometry().formatId,format)){auto selected=*this;selected.image=decoded;selected.primaryFormatId=decoded->geometry().formatId;selected.fileSystemRecognized=false;return selected;}return {};
}
DiskImageExplorer::DiskImageExplorer(MediaImageReadingService reading,MediaFileSystems::SectorRegistry fileSystems,ScpInspection::Service scp,ScpReconstruction::Reader sectors,SectorInterpretations interpretations)
 :reading_(std::move(reading)),fileSystems_(std::move(fileSystems)),scp_(std::move(scp)),sectors_(std::move(sectors)),interpretations_(std::move(interpretations)){}
std::shared_ptr<const MediaDocument> DiskImageExplorer::readDocument(const std::filesystem::path& path,std::optional<std::string> format,std::stop_token token)const{
 checkCancellation(token);SourceOptions options;options.primaryPath=utf8(path);
 if(!same(utf8(path.extension()),Scp::Constants::Extension))options.requestedFormatId=format;
 try{return reading_.readAsync(std::make_shared<MediaSource>(options),token).get();}
 catch(...){if(!format||!isUnsupportedImageRead(std::current_exception()))throw;options.requestedFormatId.reset();return reading_.readAsync(std::make_shared<MediaSource>(std::move(options)),token).get();}
}
DiskExploration DiskImageExplorer::explore(const std::filesystem::path& path,std::optional<std::string> format,const ScpInspection::Progress& progress,std::stop_token token)const{
 checkCancellation(token);if(!QFileInfo(QString::fromStdU16String(path.u16string())).isFile())throw MediaError(DiagnosticCode::AtomicOpen,{utf8(path)});
 std::shared_ptr<const MediaDocument> document;try{document=readDocument(path,format,token);}catch(...){if(!isUnsupportedImageRead(std::current_exception()))throw;return unknown(path,{},{},token);}
 return explore(std::move(document),std::move(format),progress,token);
}
DiskExploration DiskImageExplorer::exploreSectors(const std::filesystem::path& path,std::shared_ptr<const MediaDocument> document,SectorImageOwner image,std::optional<std::string> format,Scp::SharedImage capture,std::stop_token token)const{
 std::vector<ScpInspection::MatchOwner> detected;
 if(format){
  if(!same(image->geometry().formatId,*format))image=relabelSectorImage(image,*format);
  if(auto match=fileSystems_.tryRead(*image,*format,token))detected.push_back(std::make_shared<const ScpInspection::Match>(ScpInspection::Match{image,std::move(*match)}));
 }else{
  std::vector<SectorImageOwner> candidates{image};auto additional=interpretations_.additional(image,token);candidates.insert(candidates.end(),additional.begin(),additional.end());
  for(auto& match:fileSystems_.readDistinctCandidates(candidates,token))detected.push_back(std::make_shared<const ScpInspection::Match>(std::move(match)));
 }
 return make(path,std::move(document),image,std::move(detected),{image},std::move(capture),token);
}
DiskExploration DiskImageExplorer::fromScp(const std::filesystem::path& path,std::shared_ptr<const MediaDocument> document,ScpInspection::Result result,std::stop_token token)const{
 checkCancellation(token);if(!result.ranking.bestDecoded)return unknown(path,std::move(document),std::move(result.capture),token);
 auto image=result.ranking.bestRecognized?result.ranking.bestRecognized:result.ranking.bestDecoded;
 return make(path,std::move(document),std::move(image),std::move(result.detected),std::move(result.credibleImages),std::move(result.capture),token);
}
DiskExploration DiskImageExplorer::exploreScp(const std::filesystem::path& path,Scp::SharedImage capture,const ScpInspection::Progress& progress,std::stop_token token)const{return fromScp(path,{},scp_.explore(path,std::move(capture),progress,token),token);}
DiskExploration DiskImageExplorer::explore(std::shared_ptr<const MediaDocument> document,std::optional<std::string> format,const ScpInspection::Progress& progress,std::stop_token token,const DiskCaptureLoaded& loaded)const{
 checkCancellation(token);if(!document)throw MediaError(DiagnosticCode::ReaderNullDocument);DriveOperationScope activity(document->mediaSource()->options().drive.get());const auto path=std::filesystem::path(std::u8string(document->mediaSource()->primaryPath().begin(),document->mediaSource()->primaryPath().end()));
 DiskExploration result;
 if(const auto flux=dynamic_cast<const FluxRepresentation*>(&document->representation())){
  if(format&&same(*format,document->formatId()))format.reset();
  const auto capture=std::make_shared<const Scp::Image>(Scp::fromProtected(flux->image(),document->metadata(),token));if(loaded)loaded(capture);
  if(same(document->formatId(),Scp::Constants::FormatId)){
   result=fromScp(path,document,scp_.explore(path,capture,progress,token),token);
   if(format){auto selected=result.selectFormat(*format);result=selected?std::move(*selected):unknown(path,document,capture,token);}
  }else{
   ScpReconstruction::Progress units;if(progress)units=[&](const auto& value){progress({value.kind==IsoReconstruction::CollectionProgressKind::TrackDecoded?ScpInspection::ProgressKind::TrackDecoded:ScpInspection::ProgressKind::RevolutionDecoded,document->formatId(),std::size_t(value.completed),std::size_t(value.total),{},value.cylinder,value.head});};
   try{auto image=sectors_.read(flux->image(),format,units,token);result=exploreSectors(path,document,std::move(image),format,capture,token);}catch(...){if(!rejectedFlux(std::current_exception()))throw;result=unknown(path,document,capture,token);}
  }
 }else if(const auto sectors=dynamic_cast<const SectorRepresentation*>(&document->representation()))result=exploreSectors(path,document,sectors->imageOwner(),std::move(format),{},token);
 else result=unknown(path,document,{},token);
 activity.finish();return result;
}
}
