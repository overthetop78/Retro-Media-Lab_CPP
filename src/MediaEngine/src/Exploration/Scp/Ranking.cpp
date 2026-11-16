#include "Inspection.h"
#include "Constants.h"
#include "../../Models/Sectors/SectorImages.h"
#include "../../Formats/FormatIds.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaFileSystems/Interpretation.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <cmath>
#include <limits>
namespace RetroMediaLab::MediaEngine::ScpInspection {
namespace {
using namespace Constants;
using Policy=MediaFileSystems::FileSystemAlternativePolicy;
using Identity=MediaFileSystems::FileSystemInterpretationIdentity;
std::pair<std::set<std::int32_t>,std::set<std::int32_t>> dimensions(const Scp::Image& source){std::set<std::int32_t> cylinders,heads;for(const auto& track:source.tracks){cylinders.insert(track.cylinder);heads.insert(track.head);}return {std::move(cylinders),std::move(heads)};}
bool compatible(const MediaFileSystems::SectorImage& image,const Scp::Image& source){const auto [cylinders,heads]=dimensions(source);const auto minimum=std::max(std::size_t(1),std::size_t(std::floor(cylinders.size()*MinimumCylinderRatio)));return image.geometry().heads==heads.size()&&image.geometry().cylinders>=minimum&&image.geometry().cylinders<=cylinders.size();}
bool sparse(const MediaFileSystems::SectorImage& image,const Inspection& inspection,const Scp::Image& source,std::stop_token token){
 if(source.tracks.empty())return false;const auto [cylinders,heads]=dimensions(source);
 if(image.geometry().heads!=std::uint32_t(*heads.rbegin()+1)||image.geometry().cylinders<std::uint32_t(*cylinders.rbegin()+1)||image.geometry().cylinders<=cylinders.size())return false;
 std::uint64_t count=0;image.visitAvailableBlocks([&](auto){++count;return true;},token);if(count>=image.blockCount())return false;
 return std::any_of(inspection.matches.begin(),inspection.matches.end(),[&](const auto& match){return match->image->geometry().formatId==image.geometry().formatId&&Policy::isCredible(match->match.volume);});
}
bool insert(std::vector<std::string>& values,std::string value){if(std::any_of(values.begin(),values.end(),[&](const auto& old){return Primitives::ordinalIdentifierEqualsIgnoreCase(old,value);}))return false;values.push_back(std::move(value));return true;}
}
double decodeScore(const MediaFileSystems::SectorImage& image,std::stop_token token){std::uint64_t count=0;image.visitAvailableBlocks([&](auto){++count;return true;},token);return double(count)/double(std::max(std::uint64_t(1),image.blockCount()));}
Image normalizeCompatibleGeometry(Image image,const Scp::Image& source,std::stop_token token){
 checkCancellation(token);if(!image)return image;const auto [cylinders,heads]=dimensions(source);const auto& geometry=image->geometry();
 if(heads.size()!=CompatibleHeads||cylinders.size()<CompatibleCylinders||geometry.heads!=CompatibleHeads||geometry.blockSize!=CompatibleBlockSize||geometry.sectorsPerTrack!=CompatibleSectors)return image;
 std::string format;if(Primitives::asciiIdentifierStartsWithIgnoreCase(geometry.formatId,Formats::AtariStPrefix))format=Formats::AtariSt720;else if(Primitives::asciiIdentifierStartsWithIgnoreCase(geometry.formatId,Formats::IbmPrefix))format=Formats::Ibm720;else return image;
 std::vector<MediaFileSystems::SectorBlock> blocks;image->visitAvailableBlocks([&](auto number){blocks.push_back(*image->block(number,token));return true;},token);
 return sparseSectorImage({std::move(format),CompatibleBlockSize,CompatibleCylinders,CompatibleHeads,CompatibleSectors},std::move(blocks));
}
Ranking rank(std::vector<Inspection> inspections,const Scp::Image& source,std::stop_token token){
 Ranking result;double bestScore=-std::numeric_limits<double>::infinity();std::optional<MediaFileSystems::FileSystemEvidence> bestEvidence;std::vector<std::string> identities,formats;
 for(auto& inspection:inspections){
  checkCancellation(token);auto image=normalizeCompatibleGeometry(inspection.image,source,token);
  if(!image||(!compatible(*image,source)&&!sparse(*image,inspection,source,token))){result.rejected.push_back(std::move(inspection));continue;}
  const auto score=decodeScore(*image,token);if(score>bestScore){result.bestDecoded=image;bestScore=score;}
  if(score>=MinimumDecodedFormatScore&&insert(formats,image->geometry().formatId))result.decodedImages.push_back(image);
  for(const auto& match:inspection.matches){
   if(!Policy::isCredible(match->match.volume))continue;
   const auto evidence=MediaFileSystems::FileSystemEvidence::from(match->match.volume,decodeScore(*match->image,token));
   if(!bestEvidence||evidence.isBetterThan(*bestEvidence)){result.bestRecognized=match->image;result.bestFileSystem=match;bestEvidence=evidence;}
   if(insert(identities,Identity::create(match->image->geometry().formatId,match->match.volume)))result.detected.push_back(match);
  }
 }
 return result;
}
}
