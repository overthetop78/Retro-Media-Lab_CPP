#pragma once

#include "EmulationExport.h"
#include <QString>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace RetroMediaLab::Emulation {
struct NativeCoreVfsInterface {
    const char* (*getPath)(void*);
    void* (*open)(const char*, unsigned, unsigned);
    int (*close)(void*);
    std::int64_t (*size)(void*);
    std::int64_t (*tell)(void*);
    std::int64_t (*seek)(void*, std::int64_t, int);
    std::int64_t (*read)(void*, void*, std::uint64_t);
    std::int64_t (*write)(void*, const void*, std::uint64_t);
    int (*flush)(void*);
    int (*remove)(const char*);
    int (*rename)(const char*, const char*);
    std::int64_t (*truncate)(void*, std::int64_t);
    int (*stat)(const char*, std::int32_t*);
    int (*makeDirectory)(const char*);
    void* (*openDirectory)(const char*, bool);
    bool (*readDirectory)(void*);
    const char* (*directoryEntryName)(void*);
    bool (*directoryEntryIsDirectory)(void*);
    int (*closeDirectory)(void*);
};

struct NativeCoreVfsRequest {
    unsigned requiredInterfaceVersion;
    const NativeCoreVfsInterface* interface;
};

class RETROMEDIALAB_EMULATION_API NativeCoreFileSystem final {
public:
    struct ResolvedPath {
        QString path;
        bool readOnly = false;
    };

    static constexpr unsigned InterfaceVersion = 3;

    using ReadObserver = std::function<void(const QString&, std::int64_t)>;
    using PathResolver = std::function<ResolvedPath(const QString&)>;

    explicit NativeCoreFileSystem(ReadObserver observer = {}, PathResolver resolver = {});
    ~NativeCoreFileSystem();
    NativeCoreFileSystem(const NativeCoreFileSystem&) = delete;
    NativeCoreFileSystem& operator=(const NativeCoreFileSystem&) = delete;

    // The table and its borrowed strings are valid only while their owner is alive.
    bool provide(NativeCoreVfsRequest* request) const noexcept;

private:
    struct State;
    std::size_t slot_;
};
}
