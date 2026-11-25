#include "Service.h"
#include "Constants.h"
#include "../../Images/ReadRejection.h"
#include "../Disk/Projection.h"
#include "../../Formats/Tape/Common/SignalBlocks.h"
#include "../../Formats/Scp/Constants.h"
#include "../../Models/Flux/Representation.h"
#include "../../Models/Sectors/Representation.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine {
namespace {
std::string utf8(const std::filesystem::path& path){const auto text=path.u8string();return {text.begin(),text.end()};}
void report(const MediaExplorationProgressCallback& callback,MediaExplorationStage stage,std::string detail,double value,std::optional<MediaKind> kind={}){if(callback)callback({stage,std::move(detail),value,kind});}
}
MediaExplorationService::MediaExplorationService(MediaImageReadingService reading,DiskImageExplorer disks,MediaExplorer media):reading_(std::move(reading)),disks_(std::move(disks)),media_(std::move(media)){}
std::shared_ptr<const MediaDocument> MediaExplorationService::read(const std::filesystem::path& path,std::optional<std::string> format,std::stop_token token)const{
 checkCancellation(token);SourceOptions options;options.primaryPath=utf8(path);if(Primitives::isBlankIdentifier(options.primaryPath))throw MediaError(DiagnosticCode::ConversionRequest);// A filesystem interpretation is not the format of the flux container.
 if(!Primitives::ordinalIdentifierEqualsIgnoreCase(utf8(path.extension()),Scp::Constants::Extension))
     options.requestedFormatId=std::move(format);
 std::shared_ptr<const MediaDocument> document;
 try{document=reading_.readAsync(std::make_shared<MediaSource>(options),token).get();}catch(...){if(!options.requestedFormatId||!isUnsupportedImageRead(std::current_exception()))throw;options.requestedFormatId.reset();document=reading_.readAsync(std::make_shared<MediaSource>(std::move(options)),token).get();}
 checkCancellation(token);return TapeSignal::indexBlocks(std::move(document),token);
}
ExploredMediaImage MediaExplorationService::explore(const std::filesystem::path& path,std::optional<std::string> format,const MediaExplorationProgressCallback& progress,std::stop_token token)const{
 using namespace MediaExplorationConstants;report(progress,MediaExplorationStage::ReadingMedia,utf8(path.filename()),ExplorationReading);
 const auto document=read(path,std::move(format),token);report(progress,MediaExplorationStage::ReadingFileSystem,document->formatId(),ExplorationFileSystem,document->mediaKind());
 auto result=media_.exploreAsync(document,token).get();report(progress,MediaExplorationStage::RecognizingFiles,result.firstFileName.value_or(document->formatId()),RecognizingFiles,document->mediaKind());return result;
}
MediaOpeningAnalysisResult MediaExplorationService::analyze(const std::filesystem::path& path,std::optional<std::string> format,const DiskExplorationProgressCallback& diskProgress,const MediaExplorationProgressCallback& progress,std::stop_token token,bool includeFileSystems,const DiskCaptureLoadedCallback& captureLoaded)const{
 using namespace MediaExplorationConstants;report(progress,MediaExplorationStage::ReadingMedia,utf8(path.filename()),OpeningReading);
 auto document=read(path,format,token);if(!includeFileSystems)return {std::move(document),{}, {}};
 report(progress,MediaExplorationStage::ReadingFileSystem,document->formatId(),OpeningFileSystem,document->mediaKind());
 DiskCaptureLoaded loaded;if(captureLoaded)loaded=[captureLoaded](auto capture){captureLoaded(DiskProjectionAccess::capture(std::move(capture)));};
 auto diskTask=std::async(std::launch::async,[disks=disks_,document,format,diskProgress,token,loaded]{return disks.explore(document,format,diskProgress,token,loaded);});
 std::optional<DiskExploration> disk;std::optional<ExploredMediaImage> media;
 if(dynamic_cast<const FluxRepresentation*>(&document->representation())){
  disk=diskTask.get();auto decoded=document;
  if(!Primitives::ordinalIdentifierEqualsIgnoreCase(disk->image->geometry().formatId,UnknownFormat))decoded=std::make_shared<MediaDocument>(document->mediaSource(),disk->image->geometry().formatId,document->mediaKind(),std::make_shared<SectorRepresentation>(disk->image),std::vector<MediaFileSystems::VolumeDescriptor>{},std::vector<MediaFileSystems::MessageRecipe>(document->messages().begin(),document->messages().end()),document->metadata(),std::vector<std::string>(document->diagnostics().begin(),document->diagnostics().end()));
  media=media_.exploreAsync(std::move(decoded),token).get();
 }else{
  media=media_.exploreAsync(document,token).get();disk=diskTask.get();
 }
 auto publicDisk=DiskProjectionAccess::create(std::move(*disk),token);checkCancellation(token);report(progress,MediaExplorationStage::RecognizingFiles,document->formatId(),RecognizingFiles,document->mediaKind());return {std::move(document),std::move(publicDisk),std::move(media)};
}
}
