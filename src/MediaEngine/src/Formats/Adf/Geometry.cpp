#include "Internal.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Adf {
namespace Internal {
bool same(std::string_view a,std::string_view b){if(a.size()!=b.size())return false;const auto lower=[](char c){return c>=Constants::UpperFirst&&c<=Constants::UpperLast?char(c+Constants::CaseDelta):c;};return std::equal(a.begin(),a.end(),b.begin(),[&](char x,char y){return lower(x)==lower(y);});}
std::uint64_t capacity(const Profile& p){const auto& g=p.geometry;return std::uint64_t(g.blockSize)*g.cylinders*g.heads*g.sectorsPerTrack;}
Profile profile(const Constants::Specification& s){return {{std::string(s.id),s.blockSize,s.cylinders,s.heads,s.sectorsPerTrack},s.firstSector};}
bool acceptedSize(std::uint64_t size){if(size==Constants::PaddedCapacity)return true;for(const auto& p:acornGeometries())if(capacity(p)==size)return true;for(const auto& p:amigaGeometries())if(capacity(p)==size)return true;return false;}
std::string acceptedSizes(){std::string result;for(const auto& p:acornGeometries()){if(!result.empty())result+=Constants::SizeSeparator;result+=std::to_string(capacity(p));}result+=Constants::SizeSeparator;result+=std::to_string(Constants::PaddedCapacity);for(const auto& p:amigaGeometries()){result+=Constants::SizeSeparator;result+=std::to_string(capacity(p));}return result;}
}
std::vector<Profile> acornGeometries(){std::vector<Profile> result;for(const auto& s:Constants::AcornProfiles)result.push_back(Internal::profile(s));return result;}
std::vector<Profile> amigaGeometries(){std::vector<Profile> result;for(const auto& s:Constants::AmigaProfiles)result.push_back(Internal::profile(s));return result;}
std::optional<Profile> findGeometry(std::string_view id){for(const auto& p:acornGeometries())if(Internal::same(id,p.geometry.formatId))return p;for(const auto& p:amigaGeometries())if(Internal::same(id,p.geometry.formatId))return p;return {};}
bool hasDosPlusHeader(MediaFileSystems::Bytes bytes){using namespace Constants;return bytes.size()>=FatHeaderSize&&bytes[0]==DosDescriptor&&bytes[1]==FatHeaderByte&&bytes[FatHeaderLastByte]==FatHeaderByte;}
}
