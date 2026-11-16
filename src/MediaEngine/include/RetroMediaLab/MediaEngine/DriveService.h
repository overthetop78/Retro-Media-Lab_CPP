#pragma once
#include "DriveMechanics.h"
#include "MediaKind.h"
#include "Export.h"
#include <functional>
#include <memory>
namespace RetroMediaLab::MediaEngine {
class MediaDriveService final {
public:
    using Notification=std::function<void(std::shared_ptr<DriveMechanics>)>;
    RETROMEDIALAB_MEDIAENGINE_API MediaDriveService();
    RETROMEDIALAB_MEDIAENGINE_API void setAudioAvailable(Notification)const;
    RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<DriveMechanics> create(MediaKind)const;
    RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<DriveMechanics> create(std::string_view device,bool hostAudio=false)const;
    RETROMEDIALAB_MEDIAENGINE_API static std::vector<std::string> soundProfiles(MediaKind);
    RETROMEDIALAB_MEDIAENGINE_API static std::string soundProfileDisplayName(std::string_view);
private:
    std::shared_ptr<DriveMechanics> createDevice(std::string,bool)const;
    struct State;std::shared_ptr<State> state_;
};
}
