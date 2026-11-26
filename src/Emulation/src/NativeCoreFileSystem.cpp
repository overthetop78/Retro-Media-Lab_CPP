#include <RetroMediaLab/Emulation/NativeCoreFileSystem.h>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <algorithm>
#include <array>
#include <atomic>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace RetroMediaLab::Emulation {
namespace {
constexpr unsigned InterfaceVersion = 3;
constexpr unsigned ReadAccess = 1, WriteAccess = 2, UpdateExisting = 4;
constexpr int Failure = -1, DirectoryAlreadyExists = -2;
constexpr int StatInvalid = 0, StatValid = 1, StatDirectory = 2;
constexpr int SeekStart = 0, SeekCurrent = 1, SeekEnd = 2;
constexpr auto UnavailableKey = "Emulation.Error.EmulatorRejected";

// The ABI has no user data for path callbacks. Each active core gets its own entry points.
constexpr std::size_t CallbackSlotCount = 64;
std::atomic_uintptr_t nextHandle = 1;

void* newHandle() {
    return reinterpret_cast<void*>(nextHandle.fetch_add(1, std::memory_order_relaxed));
}

struct OpenFile {
    QFile file;
    QByteArray exposedPath;
    bool reads = false;
    bool writes = false;
};

struct OpenDirectory {
    OpenDirectory(const QString& path, bool includeHidden)
        : iterator(path, QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System |
            (includeHidden ? QDir::Filters{QDir::Hidden} : QDir::Filters{})) {}

    QDirIterator iterator;
    QByteArray name;
    bool isDirectory = false;
};
}

struct NativeCoreFileSystem::State {
    State(ReadObserver readObserver, PathResolver pathResolver)
        : observer(std::move(readObserver)), resolver(std::move(pathResolver)) {}

    ResolvedPath resolve(const char* utf8) const {
        const auto name = QString::fromUtf8(utf8);
        if (name.trimmed().isEmpty()) return {};
        return resolver ? resolver(name) : ResolvedPath{name, false};
    }

    const char* getPath(void* handle) {
        std::lock_guard lock(gate);
        const auto file = files.find(handle);
        return file == files.end() ? nullptr : file->second->exposedPath.constData();
    }

    void* open(const char* path, unsigned mode, unsigned) {
        const auto resolved = resolve(path);
        if (resolved.path.isEmpty() || QFileInfo(resolved.path).isDir()) return nullptr;
        auto file = std::make_unique<OpenFile>();
        file->reads = (mode & ReadAccess) != 0;
        file->writes = (mode & WriteAccess) != 0 && !resolved.readOnly;
        if (!file->reads && !file->writes) return nullptr;
        const auto absolute = QFileInfo(resolved.path).absoluteFilePath();
        file->file.setFileName(absolute);
        file->exposedPath = resolver ? QByteArray(path) : absolute.toUtf8();
        QIODevice::OpenMode flags = file->writes ? QIODevice::ReadWrite : QIODevice::ReadOnly;
        if (file->writes && (mode & UpdateExisting) == 0) {
            flags = (file->reads ? QIODevice::ReadWrite : QIODevice::WriteOnly) | QIODevice::Truncate;
        }
        if (!file->file.open(flags)) return nullptr;
        const auto handle = newHandle();
        std::lock_guard lock(gate);
        files.emplace(handle, std::move(file));
        return handle;
    }

    int close(void* handle) {
        std::lock_guard lock(gate);
        const auto file = files.extract(handle);
        if (file.empty()) return Failure;
        const auto flushed = !file.mapped()->writes || file.mapped()->file.flush();
        return flushed ? 0 : Failure;
    }

    std::int64_t size(void* handle) {
        return withFile(handle, [](OpenFile& file) { return file.file.size(); });
    }

    std::int64_t tell(void* handle) {
        return withFile(handle, [](OpenFile& file) { return file.file.pos(); });
    }

    std::int64_t seek(void* handle, std::int64_t offset, int origin) {
        return withFile(handle, [=](OpenFile& entry) {
            std::int64_t base = 0;
            switch (origin) {
            case SeekStart: break;
            case SeekCurrent: base = entry.file.pos(); break;
            case SeekEnd: base = entry.file.size(); break;
            default: return Failure;
            }
            if (base < 0 || offset < -base || offset > std::numeric_limits<std::int64_t>::max() - base) return Failure;
            return entry.file.seek(base + offset) ? 0 : Failure;
        });
    }

    std::int64_t read(void* handle, void* buffer, std::uint64_t length) {
        if (!buffer || length > std::numeric_limits<int>::max()) return Failure;
        QString path;
        const auto count = withFile(handle, [&](OpenFile& entry) -> std::int64_t {
            if (!entry.reads) return Failure;
            if (observer) path = entry.file.fileName();
            return entry.file.read(static_cast<char*>(buffer), static_cast<qint64>(length));
        });
        if (observer && count > 0) observer(path, count);
        return count;
    }

    std::int64_t write(void* handle, const void* buffer, std::uint64_t length) {
        if (!buffer || length > std::numeric_limits<int>::max()) return Failure;
        return withFile(handle, [&](OpenFile& entry) -> std::int64_t {
            if (!entry.writes) return Failure;
            const auto requested = static_cast<qint64>(length);
            return entry.file.write(static_cast<const char*>(buffer), requested) == requested ? requested : Failure;
        });
    }

    int flush(void* handle) {
        return static_cast<int>(withFile(handle, [](OpenFile& entry) { return entry.file.flush() ? 0 : Failure; }));
    }

    int remove(const char* path) {
        const auto resolved = resolve(path);
        if (resolved.path.isEmpty() || resolved.readOnly) return Failure;
        const QFileInfo file(resolved.path);
        if (file.isDir() && !file.isSymLink()) return Failure;
        return (!file.exists() && !file.isSymLink()) || QFile::remove(resolved.path) ? 0 : Failure;
    }

    int rename(const char* from, const char* to) {
        const auto source = resolve(from);
        const auto destination = resolve(to);
        if (source.path.isEmpty() || destination.path.isEmpty() || source.readOnly || destination.readOnly) return Failure;
        return QFile::rename(source.path, destination.path) ? 0 : Failure;
    }

    std::int64_t truncate(void* handle, std::int64_t length) {
        return withFile(handle, [=](OpenFile& entry) {
            if (length < 0 || !entry.writes || !entry.file.resize(length)) return Failure;
            return entry.file.pos() <= length || entry.file.seek(length) ? 0 : Failure;
        });
    }

    int stat(const char* path, std::int32_t* sizeOut) {
        if (sizeOut) *sizeOut = 0;
        const auto resolved = resolve(path);
        if (resolved.path.isEmpty()) return StatInvalid;
        const QFileInfo file(resolved.path);
        if (file.isDir()) return StatValid | StatDirectory;
        if (!file.isFile()) return StatInvalid;
        if (sizeOut) *sizeOut = static_cast<std::int32_t>(std::min<qint64>(file.size(), std::numeric_limits<std::int32_t>::max()));
        return StatValid;
    }

    int makeDirectory(const char* path) {
        const auto resolved = resolve(path);
        if (resolved.path.isEmpty() || resolved.readOnly) return Failure;
        if (QFileInfo(resolved.path).isDir()) return DirectoryAlreadyExists;
        return QDir().mkpath(resolved.path) ? 0 : Failure;
    }

    void* openDirectory(const char* path, bool includeHidden) {
        const auto resolved = resolve(path);
        if (resolved.path.isEmpty() || !QFileInfo(resolved.path).isDir()) return nullptr;
        auto directory = std::make_unique<OpenDirectory>(resolved.path, includeHidden);
        const auto handle = newHandle();
        std::lock_guard lock(gate);
        directories.emplace(handle, std::move(directory));
        return handle;
    }

    bool readDirectory(void* handle) {
        std::lock_guard lock(gate);
        const auto directory = directories.find(handle);
        if (directory == directories.end()) return false;
        auto& entry = *directory->second;
        entry.name.clear();
        entry.isDirectory = false;
        if (!entry.iterator.hasNext()) return false;
        entry.iterator.next();
        const auto info = entry.iterator.fileInfo();
        entry.name = info.fileName().toUtf8();
        entry.isDirectory = info.isDir();
        return true;
    }

    const char* directoryEntryName(void* handle) {
        std::lock_guard lock(gate);
        const auto directory = directories.find(handle);
        return directory == directories.end() || directory->second->name.isEmpty() ? nullptr : directory->second->name.constData();
    }

    bool directoryEntryIsDirectory(void* handle) {
        std::lock_guard lock(gate);
        const auto directory = directories.find(handle);
        return directory != directories.end() && directory->second->isDirectory;
    }

    int closeDirectory(void* handle) {
        std::lock_guard lock(gate);
        return directories.erase(handle) ? 0 : Failure;
    }

    template<class F>
    std::int64_t withFile(void* handle, F&& operation) {
        std::lock_guard lock(gate);
        const auto it = files.find(handle);
        return it == files.end() ? Failure : operation(*it->second);
    }

    template<std::size_t Slot, auto Method, auto Fallback, class... Args>
    static decltype(auto) invoke(Args... args) noexcept {
        using Result = decltype((std::declval<State>().*Method)(args...));
        try {
            std::shared_ptr<State> state;
            {
                std::lock_guard lock(slotsGate);
                state = callbackStates[Slot];
            }
            if (state) return (state.get()->*Method)(args...);
        } catch (...) {}
        return static_cast<Result>(Fallback);
    }

    template<std::size_t Slot>
    static constexpr NativeCoreVfsInterface table() {
        return {
            &invoke<Slot, &State::getPath, nullptr, void*>,
            &invoke<Slot, &State::open, nullptr, const char*, unsigned, unsigned>,
            &invoke<Slot, &State::close, Failure, void*>,
            &invoke<Slot, &State::size, Failure, void*>,
            &invoke<Slot, &State::tell, Failure, void*>,
            &invoke<Slot, &State::seek, Failure, void*, std::int64_t, int>,
            &invoke<Slot, &State::read, Failure, void*, void*, std::uint64_t>,
            &invoke<Slot, &State::write, Failure, void*, const void*, std::uint64_t>,
            &invoke<Slot, &State::flush, Failure, void*>,
            &invoke<Slot, &State::remove, Failure, const char*>,
            &invoke<Slot, &State::rename, Failure, const char*, const char*>,
            &invoke<Slot, &State::truncate, Failure, void*, std::int64_t>,
            &invoke<Slot, &State::stat, StatInvalid, const char*, std::int32_t*>,
            &invoke<Slot, &State::makeDirectory, Failure, const char*>,
            &invoke<Slot, &State::openDirectory, nullptr, const char*, bool>,
            &invoke<Slot, &State::readDirectory, false, void*>,
            &invoke<Slot, &State::directoryEntryName, nullptr, void*>,
            &invoke<Slot, &State::directoryEntryIsDirectory, false, void*>,
            &invoke<Slot, &State::closeDirectory, Failure, void*>,
        };
    }

    static const NativeCoreVfsInterface& interface(std::size_t slot) {
        static constexpr auto interfaces = []<std::size_t... Slots>(std::index_sequence<Slots...>) {
            return std::array{table<Slots>()...};
        }(std::make_index_sequence<CallbackSlotCount>{});
        return interfaces[slot];
    }

    std::mutex gate;
    std::unordered_map<void*, std::unique_ptr<OpenFile>> files;
    std::unordered_map<void*, std::unique_ptr<OpenDirectory>> directories;
    const ReadObserver observer;
    const PathResolver resolver;
    inline static std::mutex slotsGate;
    inline static std::array<std::shared_ptr<State>, CallbackSlotCount> callbackStates;
};

NativeCoreFileSystem::NativeCoreFileSystem(ReadObserver observer, PathResolver resolver) {
    auto state = std::make_shared<State>(std::move(observer), std::move(resolver));
    std::lock_guard lock(State::slotsGate);
    const auto available = std::ranges::find_if(State::callbackStates, [](const auto& entry) { return !entry; });
    if (available == State::callbackStates.end()) throw std::runtime_error(UnavailableKey);
    slot_ = static_cast<std::size_t>(available - State::callbackStates.begin());
    *available = std::move(state);
}

NativeCoreFileSystem::~NativeCoreFileSystem() {
    std::shared_ptr<State> state;
    {
        std::lock_guard lock(State::slotsGate);
        state = std::move(State::callbackStates[slot_]);
    }
}

bool NativeCoreFileSystem::provide(NativeCoreVfsRequest* request) const noexcept {
    if (!request || request->requiredInterfaceVersion > InterfaceVersion) return false;
    request->requiredInterfaceVersion = InterfaceVersion;
    request->interface = &State::interface(slot_);
    return true;
}
}
