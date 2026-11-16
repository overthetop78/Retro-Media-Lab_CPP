#include "Interpretations.h"
#include "InterpretationConstants.h"
#include "../../Models/Sectors/SectorImages.h"
#include "../../Formats/Raw/RawImages.h"
#include "../../Formats/St/Geometry.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaFileSystems/Fat.h>
#include <RetroMediaLab/MediaFileSystems/FatBootSectorLayout.h>
#include <RetroMediaLab/MediaFileSystems/FileSystemIds.h>
#include <algorithm>
#include <span>
namespace RetroMediaLab::MediaEngine {
namespace {
using namespace InterpretationConstants;
namespace Boot=MediaFileSystems::FatBootSectorLayout;
namespace Fs=MediaFileSystems::FileSystemIds;
bool same(std::string_view a,std::string_view b){return Primitives::asciiIdentifierEqualsIgnoreCase(a,b);}
bool prefix(std::string_view a,std::string_view b){return Primitives::asciiIdentifierStartsWithIgnoreCase(a,b);}
SectorImageOwner msx(SectorImageOwner image,std::stop_token token){
 if(prefix(image->geometry().formatId,MsxPrefix)||image->geometry().blockSize!=Boot::SectorSize||!image->containsBlock(Boot::BootLogicalBlock))return {};
 const auto boot=image->block(Boot::BootLogicalBlock,token);
 if(!boot||boot->data.size()!=Boot::SectorSize||!Raw::looksLikeMsx(boot->data,boot->data.size()))return {};
 const auto geometry=Raw::msxFromCapacity(image->blockCount()*image->geometry().blockSize,boot->data[Boot::MediaDescriptorOffset]);
 return geometry?relabelSectorImage(std::move(image),geometry->formatId):nullptr;
}
SectorImageOwner mac(SectorImageOwner image,std::string_view reader,std::stop_token token){
 checkCancellation(token);
 if((!same(reader,Fs::MacHfs)&&!same(reader,Fs::MacMfs))||image->geometry().blockSize!=Boot::SectorSize||image->blockCount()!=MacMfmSectorCount||same(image->geometry().formatId,Mac1440))return {};
 return relabelSectorImage(std::move(image),std::string(Mac1440));
}
SectorImageOwner atari(SectorImageOwner image,std::string_view reader,std::stop_token token){
 if(!same(reader,Fs::Fat12)||!prefix(image->geometry().formatId,AtariStPrefix)||!image->containsBlock(Boot::BootLogicalBlock))return {};
 const auto boot=image->block(Boot::BootLogicalBlock,token);if(!boot)return {};
 const auto geometry=MediaFileSystems::Fat::detectBpbGeometry(boot->data);if(!geometry)return {};
 std::vector<MediaFileSystems::SectorBlock> blocks;image->visitAvailableBlocks([&](auto number){if(number<geometry->totalSectors)blocks.push_back(*image->block(number,token));return true;},token);
 const auto capacity=std::uint64_t(geometry->totalSectors)*geometry->sectorSize;
 return sparseSectorImage({St::formatId(std::int64_t(capacity)),geometry->sectorSize,geometry->cylinders,geometry->heads,geometry->sectorsPerTrack},std::move(blocks),{false,capacity,geometry->totalSectors});
}
std::vector<SectorImageOwner> compatible(SectorImageOwner image,std::stop_token token){
 checkCancellation(token);std::span<const std::string_view> formats;
 switch(image->geometry().blockSize){case IsoBlockSize:formats=IsoFormats;break;case DfsBlockSize:formats=DfsFormats;break;case AdfsBlockSize:formats=AdfsFormats;break;default:break;}
 std::vector<SectorImageOwner> result;for(const auto id:formats)if(!same(image->geometry().formatId,id))result.push_back(relabelSectorImage(image,std::string(id)));return result;
}
}
SectorInterpretations::SectorInterpretations(std::vector<RecognizedImageNormalizer> normalizers,std::vector<AdditionalImagePolicy> additional):normalizers_(std::move(normalizers)),additional_(std::move(additional)){
 if(std::any_of(normalizers_.begin(),normalizers_.end(),[](const auto& policy){return !policy;})||std::any_of(additional_.begin(),additional_.end(),[](const auto& policy){return !policy;}))throw MediaError(DiagnosticCode::ConversionRequest);
}
SectorInterpretations SectorInterpretations::createDefault(std::vector<std::string> supported){
 AdditionalImagePolicy ibm=[supported=std::move(supported)](SectorImageOwner image,std::stop_token token){
  using namespace InterpretationConstants;namespace Boot=MediaFileSystems::FatBootSectorLayout;
  std::vector<SectorImageOwner> result;
  if(image->geometry().blockSize!=Boot::SectorSize||prefix(image->geometry().formatId,IbmPrefix)||!image->containsBlock(Boot::BootLogicalBlock))return result;
  const auto boot=image->block(Boot::BootLogicalBlock,token);if(!boot||boot->data.size()!=Boot::SectorSize)return result;
  const auto fat=image->containsBlock(Boot::FirstFatLogicalBlock)?image->block(Boot::FirstFatLogicalBlock,token):nullptr;
  const auto descriptor=fat&&fat->data.size()>Boot::FatMediaDescriptorDataOffset?fat->data[Boot::FatMediaDescriptorDataOffset]:Boot::UnknownMediaDescriptor;
  const auto geometry=Raw::ibmFromBoot(boot->data,descriptor);if(!geometry)return result;
  const auto supports=[&](std::string_view id){return std::any_of(supported.begin(),supported.end(),[&](const auto& known){return same(id,known);});};
  const auto physical=Raw::ibmFromCapacity(image->capacity());
  const auto id=prefix(geometry->formatId,IbmPrefix)&&supports(geometry->formatId)?geometry->formatId:physical&&supports(physical->formatId)?physical->formatId:std::string(IbmScan);
  result.push_back(relabelSectorImage(std::move(image),id));return result;
 };
 return {{mac,[](SectorImageOwner image,std::string_view reader,std::stop_token token){return same(reader,Fs::Fat12)?msx(std::move(image),token):nullptr;},atari},
  {std::move(ibm),[](SectorImageOwner image,std::stop_token token){std::vector<SectorImageOwner> result;if(auto candidate=msx(std::move(image),token))result.push_back(std::move(candidate));return result;},compatible}};
}
SectorImageOwner SectorInterpretations::normalize(SectorImageOwner image,std::string_view reader,std::stop_token token)const{
 checkCancellation(token);if(!image)throw MediaError(DiagnosticCode::ConversionRequest);
 for(const auto& policy:normalizers_){checkCancellation(token);if(auto normalized=policy(image,reader,token))return normalized;}return image;
}
std::vector<SectorImageOwner> SectorInterpretations::additional(SectorImageOwner image,std::stop_token token)const{
 checkCancellation(token);if(!image)throw MediaError(DiagnosticCode::ConversionRequest);const auto& id=image->geometry().formatId;
 if(!prefix(id,AtariStPrefix)&&!prefix(id,AtariPrefix)&&!prefix(id,IbmPrefix)&&!prefix(id,AcornDfsPrefix)&&!same(id,Imd))return {};
 std::vector<SectorImageOwner> result;for(const auto& policy:additional_){checkCancellation(token);auto candidates=policy(image,token);result.insert(result.end(),std::make_move_iterator(candidates.begin()),std::make_move_iterator(candidates.end()));}return result;
}
}
