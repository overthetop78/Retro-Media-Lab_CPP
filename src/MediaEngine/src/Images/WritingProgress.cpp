#include "WritingProgress.h"
#include "WritingProgressConstants.h"
#include "../Visualization/Providers.h"
#include "../Models/Sectors/Representation.h"
#include "../Models/Flux/Representation.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <map>
#include <set>
#include <thread>
namespace RetroMediaLab::MediaEngine {
namespace {
std::string join(const std::set<std::int64_t>& values){std::string text;for(const auto value:values){if(!text.empty())text+=WritingProgressConstants::ListSeparator;text+=std::to_string(value);}return text;}
}
void reportWriteInput(const MediaDocument& document,const std::string& target,const MediaExplorationProgressCallback& progress,std::stop_token token){
 if(!progress)return;using namespace WritingProgressConstants;checkCancellation(token);static const auto providers=Visualization::Registry::createDefault();const auto provider=providers.resolve(document);
 const auto report=[&](std::string detail,double value,std::optional<MessageInfo> message={}){progress({MediaExplorationStage::ReadingMedia,std::move(detail),value,document.mediaKind(),std::move(message)});};
 if(!provider){report(target,Complete);return;}
 const auto descriptor=provider->describe(document,token);const auto& fields=descriptor.fields();
 if(fields.progressUnit==VisualizationProgressUnit::Track&&!fields.elements.empty()){
  std::set<std::int64_t> positions,surfaces;for(const auto& element:fields.elements)positions.insert(element.fields().position);surfaces.insert(fields.surfaces.begin(),fields.surfaces.end());
  report(target,Start,MessageInfo{std::string(diagnosticKey(DiagnosticCode::RepresentationConvertingTracks)),{join(positions),join(surfaces)},{}});
 }
 const auto sectors=dynamic_cast<const SectorRepresentation*>(&document.representation());const auto flux=dynamic_cast<const FluxRepresentation*>(&document.representation());
 std::map<std::pair<std::int64_t,std::int64_t>,std::vector<std::uint64_t>> addressed;
 if(sectors)sectors->image().visitAvailableBlocks([&](auto logical){checkCancellation(token);if(const auto address=sectors->image().blockAddress(logical))addressed[{address->cylinder,address->head}].push_back(logical);return true;},token);
 for(std::size_t index=0;index<fields.elements.size();++index){
  checkCancellation(token);const auto& element=fields.elements[index].fields();
  if(element.surface){
   if(sectors){if(const auto found=addressed.find({element.position,*element.surface});found!=addressed.end())for(const auto logical:found->second){checkCancellation(token);const auto block=MediaFileSystems::requireSectorBlock(sectors->image(),logical,token);(void)block->data.size();}}
   else if(flux){const auto key=std::pair{element.position,std::int64_t(*element.surface)};const auto tracks=flux->tracks();const auto found=std::lower_bound(tracks.begin(),tracks.end(),key,[](const auto& track,const auto& address){return std::pair<std::int64_t,std::int64_t>{track.cylinder(),track.head()}<address;});if(found!=tracks.end()&&found->cylinder()==element.position&&found->head()==*element.surface){const auto count=found->bits()?found->bits()->size():found->revolutions().size();(void)count;}}
  }
  auto detail=target;std::optional<MessageInfo> message;
  if(fields.progressUnit==VisualizationProgressUnit::Track&&element.surface){detail=TrackPrefix+std::to_string(element.position)+TrackSeparator+std::to_string(*element.surface);message=MessageInfo{std::string(diagnosticKey(DiagnosticCode::RepresentationConvertingTrack)),{std::to_string(element.position),std::to_string(*element.surface)},{}};}
  report(std::move(detail),Start+Span*double(index+1)/double(fields.elements.size()),std::move(message));std::this_thread::yield();
 }
 if(fields.elements.empty())report(target,Complete);
}
}
