#pragma once
#include "AppApi.h"
#include "Shared/Presentation/IThemePresentation.h"
#include <functional>
#include <memory>
namespace RetroMediaLab {
class WindowCoordinator final {
public:
    using Factory = std::function<std::unique_ptr<IThemePresentation>()>;
    RETROMEDIALAB_APP_API explicit WindowCoordinator(Factory factory);
    RETROMEDIALAB_APP_API ~WindowCoordinator();
    RETROMEDIALAB_APP_API void rebuild();
    RETROMEDIALAB_APP_API void replace(Factory factory);
private:
    Factory factory_;
    std::unique_ptr<IThemePresentation> presentation_;
};
}
