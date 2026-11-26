#pragma once

#include "ControllerFeedback.h"
#include "NativeCoreEnvironment.h"
#include <functional>
#include <memory>
#include <span>
#include <utility>
#include <vector>

namespace RetroMediaLab::Emulation {
struct NativeCoreInputCallbacks {
    std::function<void()> poll;
    std::function<std::int16_t(unsigned port, unsigned device, unsigned index, unsigned id)> state;
};

// Call on the machine thread. The environment and its receivers outlive close().
class RETROMEDIALAB_EMULATION_API NativeCoreSession final {
public:
    using ContentFactory = std::function<std::unique_ptr<NativeCoreContent>(const NativeCoreMetadata&)>;
    using SubsystemContents = std::vector<std::unique_ptr<NativeCoreContent>>;
    using SubsystemContentFactory = std::function<SubsystemContents(const NativeCoreMetadata&)>;
    using ControllerPorts = QList<std::pair<unsigned, unsigned>>;

    NativeCoreSession(const QString& libraryPath, unsigned apiVersion, const QString& libraryName,
        NativeCoreEnvironment&, const NativeCoreCallbacks&);
    NativeCoreSession(const QString& libraryPath, unsigned apiVersion, const QString& libraryName,
        NativeCoreEnvironment&, NativeCoreInputCallbacks);
    NativeCoreSession(const QString& libraryPath, unsigned apiVersion, const QString& libraryName,
        NativeCoreEnvironment&, NativeCoreInputCallbacks, const QString& isolationDirectory);
    ~NativeCoreSession();
    NativeCoreSession(const NativeCoreSession&) = delete;
    NativeCoreSession& operator=(const NativeCoreSession&) = delete;

    const NativeCoreMetadata& metadata() const noexcept;
    void start(const ContentFactory& content = {}, const ControllerPorts& ports = {});
    // A null content entry represents an unpopulated slot in its original position.
    void startSubsystem(unsigned subsystem, const SubsystemContentFactory&, const ControllerPorts& ports = {});
    // Returns false once the core has requested shutdown.
    bool runFrame(const std::optional<HostProtocol::AudioBufferStatus>& status = {});
    void setInput(const HostProtocol::CoreInput& input);
    void publishKeyboard(const QSet<EmulationKey>& keys);
    const QList<NativeCoreOption>& options() const;
    const QList<NativeCoreOptionCategoryInfo>& optionCategories() const;
    void setOption(const QString& key, const QString& value);
    DiskStatus diskStatus() const;
    void selectDisk(int index);
    void insertDisk(const QString& path, const NativeCoreDiskControl::ContentFactory& = {});
    void insertDiskAt(int index, const QString& path, int firstImageIndex, const NativeCoreDiskControl::ContentFactory&);
    void ejectDisk();
    void reset();
    void setControllerPortDevice(unsigned port, unsigned device);
    QList<ControllerFeedbackState> controllerFeedback() const;
    unsigned region() const;
    CoreRuntimeStatus coreRuntimeStatus(std::size_t maximumStateSize) const;
    bool supportsSavedStates(std::size_t maximumSize) const;
    QByteArray saveState(std::size_t maximumSize) const;
    void loadState(QByteArrayView state, std::size_t maximumSize);
    bool restorePersistentMemory(unsigned memoryId, const QString& path, std::size_t maximumSize);
    bool savePersistentMemory(unsigned memoryId, const QString& path, std::size_t maximumSize) const;
    void close();

private:
    NativeCoreSession(const QString& libraryPath, unsigned apiVersion, const QString& libraryName,
        NativeCoreEnvironment&, const NativeCoreCallbacks&, const QString& isolationDirectory);
    class CallbackBinding;
    void startWith(const std::function<bool()>& load, const ControllerPorts&);
    void ensureRunning() const;
    const NativeCoreDiskControl& requireDiskControl() const;
    void rethrowCallbackFailure() const;
    std::span<char> persistentMemory(unsigned memoryId, std::size_t maximumSize) const;

    NativeCoreEnvironment& environment_;
    const NativeCoreCallbacks callbacks_;
    std::unique_ptr<CallbackBinding> binding_;
    std::unique_ptr<NativeCoreLibrary> library_;
    NativeCoreExports exports_;
    NativeCoreMetadata metadata_;
    std::unique_ptr<NativeCoreContent> content_;
    SubsystemContents subsystemContents_;
    std::vector<NativeCoreGameInfo> subsystemGameInfos_;
    std::size_t initialStateSize_ = 0;
    bool initialized_ = false;
    bool loaded_ = false;
};
}
