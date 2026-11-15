#include "Geometry.h"
#include "Constants.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <map>
namespace RetroMediaLab::MediaEngine::Epson {
std::vector<TrackGeometry> Geometry::allTracks()const{std::vector<TrackGeometry> result;for(std::int32_t c=0;c<cylinders;++c)for(std::int32_t h=0;h<heads;++h)result.push_back(track(c,h));return result;}
Geometry uniform(std::string id,std::int32_t cylinders,std::int32_t heads,TrackGeometry track){return {std::move(id),cylinders,heads,[track](auto,auto){return track;}};}
const std::vector<Geometry>& geometries(){using namespace Constants;static const auto values=[](){std::vector<Geometry> all;
 all.push_back(uniform(std::string(EpsonQx10_320),Cylinders,DoubleHeads,Layout320));all.push_back(uniform(std::string(EpsonQx10_400),Cylinders,DoubleHeads,Layout400));
 all.push_back({std::string(EpsonQx10Booter),BooterTracks,SingleHead,[](auto c,auto){return c==FirstCylinder?Layout320:LayoutBooter;}});
 all.push_back({std::string(EpsonQx10_399),Cylinders,DoubleHeads,[](auto c,auto h){return c==FirstCylinder&&h==0?Layout320:LayoutData;}});
 all.push_back({std::string(EpsonQx10Logo),Cylinders,DoubleHeads,[](auto c,auto){if(c==FirstCylinder||c==SecondCylinder||c==LogoSmallCylinder)return Layout320;if(c==LogoAlternateFirst||c==LogoAlternateLast)return LayoutLogo;if(c==LogoEmptyFirst||c==LogoEmptyLast)return TrackGeometry{};return LayoutData;}});
 all.push_back({std::string(EpsonQx10_396),Cylinders,DoubleHeads,[](auto c,auto){return c<=SecondCylinder?Layout320:LayoutData;}});
 for(std::size_t i=0;i<all.size();++i)for(std::size_t j=0;j<i;++j)if(all[i].cylinders==all[j].cylinders&&all[i].heads==all[j].heads&&all[i].allTracks()==all[j].allTracks())throw MediaError(DiagnosticCode::EpsonGeometryDuplicate,{all[j].formatId+std::string(Separator)+all[i].formatId});return all;}();return values;}
const Geometry& resolve(std::string_view id){for(const auto& g:geometries())if(Primitives::asciiIdentifierEqualsIgnoreCase(id,g.formatId))return g;throw MediaError(DiagnosticCode::EpsonFormat,{std::string(id)});}
std::vector<std::string> scpCandidateFormatIds(){using namespace Constants;return {std::string(EpsonQx10_396),std::string(EpsonQx10_399),std::string(EpsonQx10_320),std::string(EpsonQx10_400),std::string(EpsonQx10Logo)};}
std::optional<std::string> detect(std::span<const SectorDescriptor> sectors){using namespace Constants;std::map<std::pair<std::int32_t,std::int32_t>,std::vector<SectorDescriptor>> tracks;for(auto s:sectors)tracks[{s.cylinder,s.head}].push_back(s);if(tracks.empty())return {};
 const auto matches=[&](std::string_view id){const auto& g=resolve(id);return std::all_of(tracks.begin(),tracks.end(),[&](const auto& pair){const auto&[key,values]=pair;const auto expected=g.track(key.first,key.second);return key.first<g.cylinders&&key.second<g.heads&&expected.count>0&&values.size()==std::size_t(expected.count)&&std::all_of(values.begin(),values.end(),[&](const auto&s){return s.number>=expected.firstSector&&s.number<expected.firstSector+expected.count&&s.size==expected.sectorSize;});});};
 if(matches(EpsonQx10_320))return std::string(EpsonQx10_320);if(matches(EpsonQx10_400))return std::string(EpsonQx10_400);if(tracks.size()<=std::size_t(BooterTracks)&&std::all_of(tracks.begin(),tracks.end(),[](const auto&t){return t.first.second==0;})&&matches(EpsonQx10Booter))return std::string(EpsonQx10Booter);
 const auto small=std::count_if(tracks.begin(),tracks.end(),[](const auto&t){return std::all_of(t.second.begin(),t.second.end(),[](const auto&s){return s.size==Layout320.sectorSize;});});if(small==Small399&&matches(EpsonQx10_399))return std::string(EpsonQx10_399);if(small>=Small396&&matches(EpsonQx10_396))return std::string(EpsonQx10_396);if(small>=SmallLogo&&matches(EpsonQx10Logo))return std::string(EpsonQx10Logo);return {};
}
}
