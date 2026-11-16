#pragma once
#include "Scp.h"
#include <filesystem>
#include <future>
namespace RetroMediaLab::MediaEngine::Scp {
class RETROMEDIALAB_MEDIAENGINE_NOVTABLE CaptureReader {
public:
    virtual ~CaptureReader()=default;
    virtual void remember(const std::filesystem::path&,SharedImage)const{}
    virtual std::future<SharedImage> readFileAsync(std::filesystem::path,std::stop_token={})const=0;
};
}
