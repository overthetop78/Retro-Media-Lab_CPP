#include "Projection.h"
#include "ProjectionConstants.h"
#include "../../Images/ExplorationMapper.h"
#include "../../Images/MediaDocument.h"
#include "../../Sources/DriveOperation.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QFileInfo>
#include <QString>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::DiskProjection {
Block Cache::block(const MediaFileSystems::SectorImage& image,std::uint64_t logical)const{const auto source=images.find(&image);if(source==images.end())throw MediaError(DiagnosticCode::ReaderNullDocument);const auto found=source->second.values.find(logical);return found==source->second.values.end()?nullptr:found->second;}
std::shared_ptr<const Cache> load(const DiskExploration& disk,std::stop_token token){
 auto cache=std::make_shared<Cache>();const auto loadImage=[&](const SectorImageOwner& image){
  checkCancellation(token);if(!image)throw MediaError(DiagnosticCode::ReaderNullDocument);const auto [found,inserted]=cache->images.try_emplace(image.get());if(!inserted)return;
  image->visitAvailableBlocks([&](auto logical){checkCancellation(token);auto block=image->block(logical,token);if(!block)throw MediaError(DiagnosticCode::ReaderNullDocument);found->second.values.emplace(logical,std::move(block));found->second.order.push_back(logical);return true;},token);
 };
 loadImage(disk.image);
 for(const auto& match:disk.detected){
  loadImage(match->image);
  const auto volume=std::shared_ptr<const MediaFileSystems::FileSystemVolume>(match,&match->match.volume);
  const auto entries=MediaFileSystems::analyzeEntryViews(match->image->geometry().formatId,volume,token);
  cache->volumes.emplace(match.get(),ExplorationMapper::volumeForDisk(volume,entries));
 }
 for(const auto& image:disk.decodedImages)loadImage(image);
 return cache;
}
namespace {
bool sameDiagnostic(const DiagnosticInfo& a,const DiagnosticInfo& b){return a.message.resourceKey==b.message.resourceKey&&a.message.arguments==b.message.arguments&&a.message.translationDomain==b.message.translationDomain&&a.context==b.context&&a.level==b.level&&a.parameters==b.parameters&&a.cylinder==b.cylinder&&a.head==b.head&&a.revolution==b.revolution&&a.sector==b.sector&&a.causes.size()==b.causes.size()&&std::equal(a.causes.begin(),a.causes.end(),b.causes.begin(),sameDiagnostic);}
}
DiskImageInfo project(const DiskExploration& disk,const Cache& cache,std::stop_token token){
 checkCancellation(token);DiskImageInfo result;const auto path=disk.sourcePath.u8string();result.sourcePath.assign(path.begin(),path.end());const auto extension=disk.sourcePath.extension().u8string();auto type=QString::fromUtf8(reinterpret_cast<const char*>(extension.data()),qsizetype(extension.size()));while(type.startsWith(QLatin1Char(Constants::ExtensionDot)))type.remove(0,1);result.imageType=type.toUpper().toUtf8().toStdString();result.primaryFormatId=disk.primaryFormatId;result.metadata=disk.metadata;result.detectedFormatIds=disk.detectedFormats();
 if(disk.capture){result.version=disk.capture->header.version;result.fileSize=disk.capture->fileSize;}else{const QFileInfo info(QString::fromStdU16String(disk.sourcePath.u16string()));result.fileSize=info.isFile()?std::uint64_t(info.size()):disk.image->capacity();}
 result.container=container(disk);result.tracks=tracks(disk,cache);result.formats=formats(disk,cache,token);
 const auto selected=std::find_if(disk.detected.begin(),disk.detected.end(),[&](const auto& match){
  return match->image==disk.image;
 });
 result.volume=selected!=disk.detected.end()?cache.volumes.at(selected->get()):physicalVolume(disk);
 const auto append=[&](const std::vector<DiagnosticInfo>& warnings){for(const auto& warning:warnings)if(std::none_of(result.diagnostics.begin(),result.diagnostics.end(),[&](const auto& old){return sameDiagnostic(old,warning);}))result.diagnostics.push_back(warning);};
 append(result.volume->warnings);checkCancellation(token);return result;
}
}
namespace RetroMediaLab::MediaEngine {
struct ExploredDisk::State {DiskExploration disk;std::shared_ptr<const DiskProjection::Cache> cache;DiskImageInfo info;};
std::shared_ptr<const ExploredDisk> DiskProjectionAccess::create(DiskExploration disk,std::stop_token token){
 checkCancellation(token);if(!disk.image)throw MediaError(DiagnosticCode::ReaderNullDocument);DriveOperationScope activity(disk.document?disk.document->mediaSource()->options().drive.get():nullptr);
 auto state=std::make_shared<ExploredDisk::State>();state->disk=std::move(disk);state->cache=DiskProjection::load(state->disk,token);state->info=DiskProjection::project(state->disk,*state->cache,token);activity.finish();return std::shared_ptr<const ExploredDisk>(new ExploredDisk(std::move(state)));
}
const DiskImageInfo& ExploredDisk::info()const noexcept{return state_->info;}
std::shared_ptr<const ExploredDisk> ExploredDisk::selectFormat(std::string_view format)const{
 auto selected=state_->disk.selectFormat(format);if(!selected)return {};auto state=std::make_shared<State>();state->disk=std::move(*selected);state->cache=state_->cache;state->info=DiskProjection::project(state->disk,*state->cache,{});return std::shared_ptr<const ExploredDisk>(new ExploredDisk(std::move(state)));
}
}
