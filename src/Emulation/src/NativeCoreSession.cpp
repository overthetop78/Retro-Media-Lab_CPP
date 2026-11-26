#include <RetroMediaLab/Emulation/NativeCoreSession.h>
#include <RetroMediaLab/Emulation/InputProcessing.h>
#include <RetroMediaLab/Emulation/MachineExecution.h>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <algorithm>
#include <array>
#include <limits>
#include <mutex>
#include <stdexcept>
#include <utility>

namespace RetroMediaLab::Emulation {
namespace {
constexpr auto ErrorKey = "Emulation.Error.EmulatorRejected";
constexpr std::size_t CallbackSlotCount = 64;

std::exception_ptr rejection() {
    return std::make_exception_ptr(std::runtime_error(ErrorKey));
}
}

class NativeCoreSession::CallbackBinding final {
    struct State {
        NativeCoreEnvironment& environment;
        NativeCoreInputCallbacks input;
        HostProtocol::CoreInput inputSnapshot;
        ControllerFeedbackAccumulator feedback;
        std::mutex gate;
        std::exception_ptr failure;

        State(NativeCoreEnvironment& env, NativeCoreInputCallbacks callbacks)
            : environment(env), input(std::move(callbacks)) {}

        void recordFailure() {
            std::lock_guard lock(gate);
            if (!failure) failure = std::current_exception();
        }

        std::exception_ptr currentFailure() {
            std::lock_guard lock(gate);
            return failure;
        }
    };

    template<std::size_t Slot>
    static std::shared_ptr<State> state() {
        std::lock_guard lock(bindingsGate_);
        return activeBindings_[Slot];
    }

    template<std::size_t Slot>
    static bool environment(unsigned command, void* data) {
        const auto bound = state<Slot>();
        if (!bound) return false;
        if (bound->environment.dispatch(command, data)) return true;
        if (!data || command != static_cast<unsigned>(NativeCoreEnvironmentCommand::GetRumbleInterface)) return false;

        *static_cast<NativeCoreEnvironmentInterfaces::Rumble*>(data) = rumble<Slot>;
        return true;
    }

    template<std::size_t Slot>
    static bool rumble(unsigned port, unsigned effect, quint16 strength) {
        const auto bound = state<Slot>();
        if (!bound) return false;
        try { return bound->feedback.setState(port, effect, strength); }
        catch (...) { bound->recordFailure(); return false; }
    }

    template<std::size_t Slot>
    static void video(const void* data, unsigned width, unsigned height, std::size_t pitch) {
        if (const auto bound = state<Slot>()) bound->environment.video_.submit(data, width, height, pitch);
    }

    template<std::size_t Slot>
    static void sample(std::int16_t left, std::int16_t right) {
        if (const auto bound = state<Slot>()) bound->environment.audio_.sample(left, right);
    }

    template<std::size_t Slot>
    static std::size_t batch(const std::int16_t* data, std::size_t frames) {
        const auto bound = state<Slot>();
        return bound ? bound->environment.audio_.batch(data, frames) : 0;
    }

    template<std::size_t Slot>
    static void poll() {
        const auto bound = state<Slot>();
        if (bound && bound->input.poll) {
            try { bound->input.poll(); } catch (...) { bound->recordFailure(); }
        }
    }

    template<std::size_t Slot>
    static std::int16_t input(unsigned port, unsigned device, unsigned index, unsigned id) {
        const auto bound = state<Slot>();
        if (!bound) return 0;
        try {
            if (bound->input.state) return bound->input.state(port, device, index, id);
            std::lock_guard lock(bound->gate);
            return InputProcessing::nativeInputState(bound->inputSnapshot, port, device, index, id);
        } catch (...) {
            bound->recordFailure();
        }
        return 0;
    }

public:
    CallbackBinding(NativeCoreEnvironment& environment, NativeCoreInputCallbacks callbacks) {
        auto bound = std::make_shared<State>(environment, std::move(callbacks));
        std::lock_guard lock(bindingsGate_);
        const auto available = std::ranges::find_if(activeBindings_, [](const auto& item) { return !item; });
        if (available == activeBindings_.end()) std::rethrow_exception(rejection());
        slot_ = static_cast<std::size_t>(available - activeBindings_.begin());
        *available = std::move(bound);
    }

    ~CallbackBinding() {
        std::shared_ptr<State> released;
        {
            std::lock_guard lock(bindingsGate_);
            released = std::move(activeBindings_[slot_]);
        }
    }

    const NativeCoreCallbacks& callbacks() const {
        // The native callback signature carries no user data.
        static constexpr auto tables = []<std::size_t... Slots>(std::index_sequence<Slots...>) {
            return std::array{NativeCoreCallbacks{environment<Slots>, video<Slots>, sample<Slots>,
                batch<Slots>, poll<Slots>, input<Slots>}...};
        }(std::make_index_sequence<CallbackSlotCount>{});
        return tables[slot_];
    }

    void setInput(const HostProtocol::CoreInput& input) const {
        const auto bound = bindingState();
        std::lock_guard lock(bound->gate);
        bound->inputSnapshot = input;
    }

    QList<ControllerFeedbackState> feedback() const {
        return bindingState()->feedback.snapshot();
    }

    void resetFeedbackPort(unsigned port) const {
        if (port < ControllerFeedbackCommands::MaximumPortCount)
            bindingState()->feedback.resetPort(static_cast<int>(port));
    }

    void resetFeedback() const { bindingState()->feedback.reset(); }

    std::exception_ptr failure() const {
        return bindingState()->currentFailure();
    }

private:
    std::shared_ptr<State> bindingState() const {
        std::shared_ptr<State> bound;
        {
            std::lock_guard lock(bindingsGate_);
            bound = activeBindings_[slot_];
        }
        return bound;
    }

    inline static std::mutex bindingsGate_;
    inline static std::array<std::shared_ptr<State>, CallbackSlotCount> activeBindings_;
    std::size_t slot_;
};

NativeCoreSession::NativeCoreSession(const QString& libraryPath, unsigned apiVersion, const QString& libraryName,
    NativeCoreEnvironment& environment, const NativeCoreCallbacks& callbacks)
    : NativeCoreSession(libraryPath, apiVersion, libraryName, environment, callbacks, {}) {}

NativeCoreSession::NativeCoreSession(const QString& libraryPath, unsigned apiVersion, const QString& libraryName,
    NativeCoreEnvironment& environment, const NativeCoreCallbacks& callbacks, const QString& isolationDirectory)
    : environment_(environment), callbacks_(callbacks),
      library_(std::make_unique<NativeCoreLibrary>(libraryPath, isolationDirectory)),
      exports_(library_->resolveExports(false)), metadata_(library_->inspect(exports_, apiVersion, libraryName)) {}

NativeCoreSession::NativeCoreSession(const QString& libraryPath, unsigned apiVersion, const QString& libraryName,
    NativeCoreEnvironment& environment, NativeCoreInputCallbacks input)
    : NativeCoreSession(libraryPath, apiVersion, libraryName, environment, std::move(input), {}) {}

NativeCoreSession::NativeCoreSession(const QString& libraryPath, unsigned apiVersion, const QString& libraryName,
    NativeCoreEnvironment& environment, NativeCoreInputCallbacks input, const QString& isolationDirectory)
    : NativeCoreSession(libraryPath, apiVersion, libraryName, environment, NativeCoreCallbacks{}, isolationDirectory) {
    binding_ = std::make_unique<CallbackBinding>(environment, std::move(input));
}

NativeCoreSession::~NativeCoreSession() {
    try { close(); } catch (...) {}
}

const NativeCoreMetadata& NativeCoreSession::metadata() const noexcept { return metadata_; }

void NativeCoreSession::start(const ContentFactory& createContent, const ControllerPorts& ports) {
    startWith([&] {
        if (createContent) content_ = createContent(metadata_);
        if (!content_ && !environment_.supportsNoGame()) std::rethrow_exception(rejection());
        return exports_.loadGame(content_ ? &content_->gameInfo() : nullptr);
    }, ports);
}

void NativeCoreSession::startSubsystem(unsigned subsystem, const SubsystemContentFactory& createContent,
    const ControllerPorts& ports) {
    startWith([&] {
        const auto load = library_->resolveSpecialContent();
        if (!createContent) std::rethrow_exception(rejection());
        subsystemContents_ = createContent(metadata_);
        if (subsystemContents_.empty()) std::rethrow_exception(rejection());
        subsystemGameInfos_.reserve(subsystemContents_.size());
        for (const auto& slot : subsystemContents_)
            subsystemGameInfos_.push_back(slot ? slot->gameInfo() : NativeCoreGameInfo{});
        return load(subsystem, subsystemGameInfos_.data(), subsystemGameInfos_.size());
    }, ports);
}

void NativeCoreSession::startWith(const std::function<bool()>& load, const ControllerPorts& ports) {
    if (!library_ || initialized_) std::rethrow_exception(rejection());
    try {
        installNativeCoreCallbacks(exports_, binding_ ? binding_->callbacks() : callbacks_);
        rethrowCallbackFailure();
        exports_.initialize();
        initialized_ = true;
        rethrowCallbackFailure();

        loaded_ = load();
        rethrowCallbackFailure();
        if (!loaded_) std::rethrow_exception(rejection());

        initialStateSize_ = exports_.getSerializedSize();
        for (const auto& [port, device] : ports) exports_.setControllerPortDevice(port, device);
        NativeCoreAvInfo info{};
        exports_.getSystemAvInfo(&info);
        rethrowCallbackFailure();
        environment_.applyAvInfo(info);
    } catch (...) {
        try { close(); } catch (...) {}
        throw;
    }
}

void NativeCoreSession::ensureRunning() const {
    if (!loaded_) std::rethrow_exception(rejection());
}

void NativeCoreSession::rethrowCallbackFailure() const {
    environment_.rethrowCallbackFailure();
    if (binding_) {
        if (const auto failure = binding_->failure()) std::rethrow_exception(failure);
    }
}

bool NativeCoreSession::runFrame(const std::optional<HostProtocol::AudioBufferStatus>& status) {
    ensureRunning();
    if (environment_.shutdownRequested()) return false;
    environment_.publishAudioStatus(status);
    exports_.run();
    rethrowCallbackFailure();
    return !environment_.shutdownRequested();
}

void NativeCoreSession::setInput(const HostProtocol::CoreInput& input) {
    ensureRunning();
    if (binding_) binding_->setInput(input);
    QSet<EmulationKey> keys;
    keys.reserve(input.keys.size());
    for (const auto key : input.keys) keys.insert(static_cast<EmulationKey>(key));
    publishKeyboard(keys);
}

void NativeCoreSession::publishKeyboard(const QSet<EmulationKey>& keys) {
    ensureRunning();
    environment_.publishKeyboard(keys, InputProcessing::nativeKeyboardCodes());
    rethrowCallbackFailure();
}

const QList<NativeCoreOption>& NativeCoreSession::options() const {
    ensureRunning();
    return environment_.options();
}

const QList<NativeCoreOptionCategoryInfo>& NativeCoreSession::optionCategories() const {
    ensureRunning();
    return environment_.optionCategories();
}

void NativeCoreSession::setOption(const QString& key, const QString& value) {
    ensureRunning();
    environment_.setOption(key, value);
    rethrowCallbackFailure();
}

const NativeCoreDiskControl& NativeCoreSession::requireDiskControl() const {
    ensureRunning();
    const auto* disks = environment_.interfaces_.disks;
    if (!disks) std::rethrow_exception(rejection());
    return *disks;
}

DiskStatus NativeCoreSession::diskStatus() const {
    ensureRunning();
    const auto* disks = environment_.interfaces_.disks;
    if (!disks) return {};
    auto status = disks->status();
    rethrowCallbackFailure();
    return status;
}

void NativeCoreSession::selectDisk(int index) {
    requireDiskControl().select(index);
    rethrowCallbackFailure();
}

void NativeCoreSession::insertDisk(const QString& path, const NativeCoreDiskControl::ContentFactory& createContent) {
    requireDiskControl().insert(path, createContent);
    rethrowCallbackFailure();
}

void NativeCoreSession::insertDiskAt(int index, const QString& path, int firstImageIndex,
    const NativeCoreDiskControl::ContentFactory& createContent) {
    requireDiskControl().insertAt(index, path, firstImageIndex, createContent);
    rethrowCallbackFailure();
}

void NativeCoreSession::ejectDisk() {
    requireDiskControl().eject();
    rethrowCallbackFailure();
}

void NativeCoreSession::reset() {
    ensureRunning();
    exports_.reset();
    rethrowCallbackFailure();
    if (binding_) binding_->resetFeedback();
    environment_.clearOutputs();
}

void NativeCoreSession::setControllerPortDevice(unsigned port, unsigned device) {
    ensureRunning();
    if (binding_) binding_->resetFeedbackPort(port);
    exports_.setControllerPortDevice(port, device);
    rethrowCallbackFailure();
}

QList<ControllerFeedbackState> NativeCoreSession::controllerFeedback() const {
    return loaded_ && binding_ ? binding_->feedback() : QList<ControllerFeedbackState>{};
}

unsigned NativeCoreSession::region() const {
    ensureRunning();
    const auto result = exports_.getRegion();
    rethrowCallbackFailure();
    return result;
}

CoreRuntimeStatus NativeCoreSession::coreRuntimeStatus(std::size_t maximumStateSize) const {
    CoreRuntimeStatus result;
    if (!loaded_) return result;
    result.supportsSaveStates = supportsSavedStates(maximumStateSize);
    switch (region()) {
    case 0: result.region = VideoStandard::Ntsc; break;
    case 1: result.region = VideoStandard::Pal; break;
    default: break;
    }
    result.bufferedAudioFrames = environment_.audio_.bufferedFrames();
    result.audioOverrunCount = environment_.audio_.overrunCount();
    result.audioUnderrunCount = environment_.audio_.underrunCount();
    return result;
}

bool NativeCoreSession::supportsSavedStates(std::size_t maximumSize) const {
    return loaded_ && isSupportedStateSize(initialStateSize_, maximumSize);
}

QByteArray NativeCoreSession::saveState(std::size_t maximumSize) const {
    ensureRunning();
    auto result = serializeCoreState(supportsSavedStates(maximumSize), maximumSize, exports_.getSerializedSize,
        exports_.serialize, rejection, rejection);
    rethrowCallbackFailure();
    return result;
}

void NativeCoreSession::loadState(QByteArrayView state, std::size_t maximumSize) {
    ensureRunning();
    unserializeCoreState(supportsSavedStates(maximumSize), maximumSize, state, exports_.unserialize, rejection, rejection);
    rethrowCallbackFailure();
    if (binding_) binding_->resetFeedback();
    environment_.clearOutputs();
}

std::span<char> NativeCoreSession::persistentMemory(unsigned memoryId, std::size_t maximumSize) const {
    ensureRunning();
    const auto size = exports_.getMemorySize(memoryId);
    rethrowCallbackFailure();
    if (size == 0) return {};
    if (size > maximumSize || size > static_cast<std::size_t>(std::numeric_limits<qsizetype>::max()))
        std::rethrow_exception(rejection());
    auto* memory = static_cast<char*>(exports_.getMemoryData(memoryId));
    rethrowCallbackFailure();
    if (!memory) std::rethrow_exception(rejection());
    return {memory, size};
}

bool NativeCoreSession::restorePersistentMemory(unsigned memoryId, const QString& path, std::size_t maximumSize) {
    const auto memory = persistentMemory(memoryId, maximumSize);
    if (memory.empty()) return false;
    if (path.isEmpty()) std::rethrow_exception(rejection());
    if (!QFileInfo::exists(path)) return false;
    QFile file(path);
    const auto size = static_cast<qsizetype>(memory.size());
    if (!file.open(QIODevice::ReadOnly) || file.size() != size) std::rethrow_exception(rejection());
    const auto data = file.read(size);
    if (data.size() != size || !file.atEnd() || file.error() != QFileDevice::NoError)
        std::rethrow_exception(rejection());
    std::copy(data.cbegin(), data.cend(), memory.begin());
    return true;
}

bool NativeCoreSession::savePersistentMemory(unsigned memoryId, const QString& path, std::size_t maximumSize) const {
    const auto memory = persistentMemory(memoryId, maximumSize);
    if (memory.empty()) return false;
    if (path.isEmpty() || !QDir().mkpath(QFileInfo(path).absolutePath())) std::rethrow_exception(rejection());
    const auto size = static_cast<qsizetype>(memory.size());
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(memory.data(), size) != size || !file.commit())
        std::rethrow_exception(rejection());
    return true;
}

void NativeCoreSession::close() {
    if (!library_) return;
    const auto loaded = std::exchange(loaded_, false);
    const auto initialized = std::exchange(initialized_, false);
    cleanupCore(loaded, initialized, exports_.unloadGame, exports_.deinitialize,
        [this] {
            environment_.clearBorrowedReferences();
            subsystemGameInfos_.clear();
            subsystemContents_.clear();
            content_.reset();
            binding_.reset();
            environment_.clearOutputs();
        }, [this] { library_.reset(); });
}
}
