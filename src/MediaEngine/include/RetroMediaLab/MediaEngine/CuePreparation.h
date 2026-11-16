#pragma once
#include "Export.h"
#include <cstdint>
#include <exception>
#include <filesystem>
#include <functional>
#include <future>
#include <memory>
#include <ostream>
#include <span>
#include <stop_token>
#include <string>
#include <vector>
namespace RetroMediaLab::MediaEngine {
class CueDocument final {
public:
    RETROMEDIALAB_MEDIAENGINE_API static CueDocument parse(std::span<const std::string>,std::stop_token={});
    RETROMEDIALAB_MEDIAENGINE_API static CueDocument read(const std::filesystem::path&,std::stop_token={});
    RETROMEDIALAB_MEDIAENGINE_API std::vector<std::string> declaredFiles()const;
    RETROMEDIALAB_MEDIAENGINE_API bool isSingleBinaryRawAudio(int maximumTracks)const;
    RETROMEDIALAB_MEDIAENGINE_API CueDocument withSingleFile(std::string path)const;
    // The caller retains ownership of the stream; this method never closes it.
    RETROMEDIALAB_MEDIAENGINE_API void write(std::ostream&,std::stop_token={})const;
private:
    struct Impl;std::shared_ptr<const Impl> impl_;
    const Impl& require()const;
    explicit CueDocument(std::shared_ptr<const Impl> value):impl_(std::move(value)){}
};
using CueStageFile=std::function<std::filesystem::path(const std::filesystem::path&)>;
using CueUnsupported=std::function<std::exception_ptr()>;
// stageFile owns its copies and directory. It must preserve the original files.
// This service replaces staged WAVE copies with raw PCM and publishes their CUE atomically.
RETROMEDIALAB_MEDIAENGINE_API std::future<void> prepareCueWaveFilesAsync(std::filesystem::path sourceCue,std::filesystem::path stagedCue,
    CueStageFile stageFile,std::int32_t copyBufferBytes,std::stop_token={},CueUnsupported={});
}
