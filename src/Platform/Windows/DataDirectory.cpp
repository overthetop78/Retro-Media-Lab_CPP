#include <RetroMediaLab/Platform/DataDirectory.h>
#include <ErrorKeys.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shlobj.h>
#include <memory>
#include <stdexcept>
namespace RetroMediaLab::Platform {
std::filesystem::path userDataDirectory() {
    PWSTR directory=nullptr;
    const auto result=SHGetKnownFolderPath(FOLDERID_RoamingAppData,KF_FLAG_DEFAULT,nullptr,&directory);
    const std::unique_ptr<wchar_t,decltype(&CoTaskMemFree)> owned(directory,&CoTaskMemFree);
    if(FAILED(result)||!directory||!*directory)throw std::runtime_error(ErrorKeys::CannotResolveDataDirectory);
    return std::filesystem::path(directory);
}
}
