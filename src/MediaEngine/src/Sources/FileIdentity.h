#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <filesystem>
#include <string>
namespace RetroMediaLab::MediaEngine {
struct FileIdentity {
    std::filesystem::path path;
    std::string key;
    bool exists=false;
    std::uintmax_t length=0;
    std::filesystem::file_time_type modified{};
    bool operator==(const FileIdentity& other)const{return key==other.key&&exists==other.exists&&length==other.length&&modified==other.modified;}
};
RETROMEDIALAB_MEDIAENGINE_API FileIdentity fileIdentity(const std::filesystem::path&);
}
