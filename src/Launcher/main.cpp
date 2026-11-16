#include <ProcessConstants.h>
#include <StorageProbeConstants.h>
#include "LauncherConstants.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <ErrorDialog.h>
#include <shellapi.h>
#include <filesystem>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>
namespace {
struct WindowsError { DWORD code; std::wstring context; };
struct SearchDirectory {
    DLL_DIRECTORY_COOKIE cookie = nullptr;
    explicit SearchDirectory(const std::filesystem::path& path) {
        auto native = path;
        native.make_preferred();
        cookie = AddDllDirectory(native.c_str());
        if (!cookie) throw WindowsError{GetLastError(), path.wstring()};
    }
    SearchDirectory(SearchDirectory&& other) noexcept : cookie(std::exchange(other.cookie, nullptr)) {}
    SearchDirectory(const SearchDirectory&) = delete;
    ~SearchDirectory() { if (cookie) RemoveDllDirectory(cookie); }
};
struct Library {
    HMODULE module = nullptr;
    explicit Library(const std::filesystem::path& path) {
        auto native = path;
        native.make_preferred();
        module = LoadLibraryExW(native.c_str(), nullptr, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_USER_DIRS | LOAD_LIBRARY_SEARCH_SYSTEM32);
        if (!module) throw WindowsError{GetLastError(), path.wstring()};
    }
    Library(const Library&) = delete;
    ~Library() { FreeLibrary(module); }
};
struct LocalFreeDeleter { void operator()(void* memory) const noexcept { if (memory) LocalFree(memory); } };
void report(const WindowsError& error) {
    const auto language = RetroMediaLab::Errors::systemLanguage();
    const auto& context = error.context;
    const auto count = WideCharToMultiByte(CP_UTF8, 0, context.data(), static_cast<int>(context.size()), nullptr, 0, nullptr, nullptr);
    std::string detail(count, ::RetroMediaLab::LauncherConstants::NullCharacter);
    if (count) WideCharToMultiByte(CP_UTF8, 0, context.data(), static_cast<int>(context.size()), detail.data(), count, nullptr, nullptr);
    RetroMediaLab::Errors::show(language, RetroMediaLab::Errors::nativeFailure(language, error.code, detail));
}
}
int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    try {
        std::wstring executable(::RetroMediaLab::LauncherConstants::Process::ExecutablePathCapacity, ::RetroMediaLab::LauncherConstants::WideNullCharacter);
        const auto length = GetModuleFileNameW(nullptr, executable.data(), static_cast<DWORD>(executable.size()));
        if (!length || length >= executable.size()) throw WindowsError{ERROR_BAD_PATHNAME, ::RetroMediaLab::LauncherConstants::WideEmpty};
        executable.resize(length);
        const auto root = std::filesystem::path(executable).parent_path();
        if (!SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_USER_DIRS | LOAD_LIBRARY_SEARCH_SYSTEM32))
            throw WindowsError{GetLastError(), root.wstring()};
        std::vector<SearchDirectory> directories;
        for (const auto* name : {::RetroMediaLab::LauncherConstants::WideLib, ::RetroMediaLab::LauncherConstants::WideLibQt6, ::RetroMediaLab::LauncherConstants::WideLibDirectX, ::RetroMediaLab::LauncherConstants::WideLibOpenGL, ::RetroMediaLab::LauncherConstants::WideLibFFmpeg}) {
            const auto path = root / name;
            if (std::filesystem::is_directory(path)) directories.emplace_back(path);
        }
        int argc = 0;
        auto** wideArguments = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (!wideArguments) throw WindowsError{GetLastError(), ::RetroMediaLab::LauncherConstants::WideEmpty};
        const std::unique_ptr<void, LocalFreeDeleter> ownedArguments(wideArguments);
        std::vector<std::string> storage;
        storage.reserve(argc);
        for (int index = 0; index < argc; ++index) {
            const auto count = WideCharToMultiByte(CP_UTF8, 0, wideArguments[index], -1, nullptr, 0, nullptr, nullptr);
            if (!count) throw WindowsError{GetLastError(), ::RetroMediaLab::LauncherConstants::WideEmpty};
            std::string argument(count, ::RetroMediaLab::LauncherConstants::NullCharacter);
            WideCharToMultiByte(CP_UTF8, 0, wideArguments[index], -1, argument.data(), count, nullptr, nullptr);
            storage.push_back(std::move(argument));
        }
        // argv[0] is always an absolute path for locating the bundled resources/plugins.
        const auto count = WideCharToMultiByte(CP_UTF8, 0, executable.c_str(), -1, nullptr, 0, nullptr, nullptr);
        storage.at(0).resize(count);
        WideCharToMultiByte(CP_UTF8, 0, executable.c_str(), -1, storage[0].data(), count, nullptr, nullptr);
        std::vector<char*> arguments;
        for (auto& argument : storage) arguments.push_back(argument.data());
        arguments.push_back(nullptr);
        const bool probe = argc > 1 && std::strcmp(arguments[1], ::RetroMediaLab::StorageProbeConstants::Mode) == 0;
        Library application(root / (probe ? ::RetroMediaLab::LauncherConstants::WideLibInfrastructureDll : ::RetroMediaLab::LauncherConstants::WideLibAppDll));
        using Entry = int (__cdecl*)(int, char**);
        const auto entry = reinterpret_cast<Entry>(GetProcAddress(application.module, probe ? ::RetroMediaLab::LauncherConstants::ProbeEntry : ::RetroMediaLab::LauncherConstants::RetroMediaLabRun));
        if (!entry) throw WindowsError{GetLastError(), probe ? ::RetroMediaLab::LauncherConstants::WideProbeEntry : ::RetroMediaLab::LauncherConstants::WideRetroMediaLabRun};
        return entry(argc, arguments.data());
    } catch (const WindowsError& error) { report(error); }
      catch (...) { report({ERROR_GEN_FAILURE, ::RetroMediaLab::LauncherConstants::WideRetroMediaLab}); }
    return RetroMediaLab::ProcessConstants::Failure;
}
