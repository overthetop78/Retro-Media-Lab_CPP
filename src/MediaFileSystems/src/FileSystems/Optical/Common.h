#pragma once
#include <RetroMediaLab/MediaFileSystems/MediaDocument.h>
#include <RetroMediaLab/MediaFileSystems/FileSystemModels.h>
#include <QString>
#include <set>
namespace RetroMediaLab::MediaFileSystems::Optical {
Bytes slice(Bytes,std::size_t,std::size_t);
std::shared_ptr<const OpticalTrack> resolve(const ImageDocument&,const VolumeDescriptor&,DiagnosticCode required,DiagnosticCode absent);
std::vector<std::uint8_t> sector(const OpticalTrack&,std::int64_t,std::stop_token);
std::string ascii(Bytes);
QString utf16be(Bytes);
std::string utf8OrLatin1(Bytes);
std::uint32_t add(std::uint32_t,std::uint32_t);
int integer(std::uint64_t);
bool probeFailure(const MediaError&,bool overflow=true);
class ActiveExtent final {
public:
    ActiveExtent(std::set<std::uint32_t>& values,std::uint32_t extent):values_(values),extent_(extent),inserted_(values.insert(extent).second){}
    ~ActiveExtent(){if(inserted_)values_.erase(extent_);}
    explicit operator bool()const noexcept{return inserted_;}
    ActiveExtent(const ActiveExtent&)=delete;
    ActiveExtent& operator=(const ActiveExtent&)=delete;
private:
    std::set<std::uint32_t>& values_;std::uint32_t extent_;bool inserted_;
};
}
