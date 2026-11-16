#pragma once
#include "Constants.h"
#include "../BinCue/Sheet.h"
namespace RetroMediaLab::MediaEngine::CloneCd {
struct TocEntry {
    int entryNumber=0,sessionNumber=0,point=0,adr=0,control=0,trackNumber=0;
    int absoluteMinute=0,absoluteSecond=0,absoluteFrame=0;
    std::int64_t absoluteLba=0;
    int zero=0,pointMinute=0,pointSecond=0,pointFrame=0;
    std::int64_t pointLba=0;
};
using SessionProperties=std::map<std::string,int,OpticalMetadataLess>;
using Sessions=std::map<int,SessionProperties>;
class Descriptor final {
public:
    RETROMEDIALAB_MEDIAENGINE_API Descriptor(int version,int sessionCount,std::optional<std::string> catalog,std::vector<TocEntry>,std::vector<BinCue::TrackDeclaration>,Sessions);
    int version()const noexcept{return version_;}int sessionCount()const noexcept{return sessionCount_;}
    const std::optional<std::string>& catalog()const noexcept{return catalog_;}
    std::span<const TocEntry> entries()const noexcept{return entries_;}
    std::span<const BinCue::TrackDeclaration> tracks()const noexcept{return tracks_;}
    const Sessions& sessions()const noexcept{return sessions_;}
private:
    int version_,sessionCount_;std::optional<std::string> catalog_;std::vector<TocEntry> entries_;std::vector<BinCue::TrackDeclaration> tracks_;Sessions sessions_;
};
RETROMEDIALAB_MEDIAENGINE_API Descriptor parseDescriptor(std::span<const std::string>,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API Descriptor readDescriptor(const std::filesystem::path&,std::stop_token={});
}
