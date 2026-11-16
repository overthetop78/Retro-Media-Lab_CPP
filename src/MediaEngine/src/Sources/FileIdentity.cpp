#include "FileIdentity.h"
#include <RetroMediaLab/MediaFileSystems/Diagnostics.h>
#include <QString>
namespace RetroMediaLab::MediaEngine {
namespace {
std::string utf8(const std::filesystem::path& path){const auto value=path.u8string();return {value.begin(),value.end()};}
}
FileIdentity fileIdentity(const std::filesystem::path& path){
    if(path.empty()||path.native().find(std::filesystem::path::value_type{})!=std::filesystem::path::string_type::npos)throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::OpenFailed,{utf8(path)});
    std::error_code error;FileIdentity result;result.path=std::filesystem::absolute(path,error).lexically_normal();
    if(error)throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::OpenFailed,{utf8(path)});
    result.key=utf8(result.path);
#if defined(_WIN32)
    result.key=QString::fromStdString(result.key).toCaseFolded().toUtf8().toStdString();
#endif
    if(!std::filesystem::is_regular_file(result.path,error)||error)return result;
    const auto length=std::filesystem::file_size(result.path,error);if(error)return result;
    const auto modified=std::filesystem::last_write_time(result.path,error);if(error)return result;
    result.exists=true;result.length=length;result.modified=modified;return result;
}
}
