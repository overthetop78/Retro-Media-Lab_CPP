#pragma once
#include "DriveMechanics.h"
#include "MediaKind.h"
#include "Export.h"
#include <functional>
#include <memory>
namespace RetroMediaLab::MediaEngine {
class MediaDriveService final {
public:
    using Notification=std::function<void(DriveMechanics&)>;
    RETROMEDIALAB_MEDIAENGINE_API MediaDriveService();
    RETROMEDIALAB_MEDIAENGINE_API void setAudioAvailable(Notification)const;
    RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<DriveMechanics> create(MediaKind)const;
private:
    struct State;std::shared_ptr<State> state_;
};
}
