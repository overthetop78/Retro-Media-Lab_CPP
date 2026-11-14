#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#include <filesystem>
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
    wchar_t* buffer = nullptr;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, error.code, 0, reinterpret_cast<LPWSTR>(&buffer), 0, nullptr);
    const std::unique_ptr<void, LocalFreeDeleter> owned(buffer);
    const std::wstring message = (buffer ? std::wstring(buffer) : std::to_wstring(error.code)) + L"\n" + error.context;
    MessageBoxW(nullptr, message.c_str(), L"Retro Media Lab", MB_OK | MB_ICONERROR);
}
}
int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    try {
        std::wstring executable(32768, L'\0');
        const auto length = GetModuleFileNameW(nullptr, executable.data(), static_cast<DWORD>(executable.size()));
        if (!length || length >= executable.size()) throw WindowsError{ERROR_BAD_PATHNAME, L""};
        executable.resize(length);
        const auto root = std::filesystem::path(executable).parent_path();
        if (!SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_USER_DIRS | LOAD_LIBRARY_SEARCH_SYSTEM32))
            throw WindowsError{GetLastError(), root.wstring()};
        std::vector<SearchDirectory> directories;
        for (const auto* name : {L"lib", L"lib/Qt6", L"lib/DirectX", L"lib/OpenGL"}) {
            const auto path = root / name;
            if (std::filesystem::is_directory(path)) directories.emplace_back(path);
        }
        Library application(root / L"lib/App.dll");
        using Entry = int (__cdecl*)(int, char**);
        const auto entry = reinterpret_cast<Entry>(GetProcAddress(application.module, "RetroMediaLabRun"));
        if (!entry) throw WindowsError{GetLastError(), L"RetroMediaLabRun"};
        int argc = 0;
        auto** wideArguments = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (!wideArguments) throw WindowsError{GetLastError(), L""};
        const std::unique_ptr<void, LocalFreeDeleter> ownedArguments(wideArguments);
        std::vector<std::string> storage;
        storage.reserve(argc);
        for (int index = 0; index < argc; ++index) {
            const auto count = WideCharToMultiByte(CP_UTF8, 0, wideArguments[index], -1, nullptr, 0, nullptr, nullptr);
            if (!count) throw WindowsError{GetLastError(), L""};
            std::string argument(count, '\0');
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
        return entry(argc, arguments.data());
    } catch (const WindowsError& error) { report(error); }
      catch (...) { report({ERROR_GEN_FAILURE, L"Retro Media Lab"}); }
    return 1;
}
