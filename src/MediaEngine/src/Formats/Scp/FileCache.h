#pragma once
#include "Scp.h"
#include <filesystem>
#include <future>
namespace RetroMediaLab::MediaEngine::Scp {
class FileCache final {
public:
    // A caller's cancellation stops its wait; the load receives only the cache-owned lifetime token.
    using Loader=std::function<SharedImage(const std::filesystem::path&,std::stop_token)>;
    RETROMEDIALAB_MEDIAENGINE_API FileCache();
    RETROMEDIALAB_MEDIAENGINE_API void remember(const std::filesystem::path&,SharedImage)const;
    RETROMEDIALAB_MEDIAENGINE_API std::future<SharedImage> getOrAddAsync(std::filesystem::path,Loader,std::stop_token={})const;
private:struct State;std::shared_ptr<State> state_;
};
}
