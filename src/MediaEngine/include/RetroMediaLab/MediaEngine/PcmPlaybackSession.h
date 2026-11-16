#pragma once
#include "Export.h"
#include <cstdint>
#include <filesystem>
#include <future>
#include <memory>
#include <span>
#include <stop_token>
namespace RetroMediaLab::MediaEngine {
class MediaDocument;
// Sample-positioned access to decoded interleaved PCM, without an output device.
// Calls on one session must be serialized by its owner.
class PcmPlaybackSession final {
public:
 RETROMEDIALAB_MEDIAENGINE_API static std::future<std::unique_ptr<PcmPlaybackSession>> openAsync(std::filesystem::path,std::stop_token={});
 // Transfers responsibility for releasing the document's PCM sources and resetting its drive.
 RETROMEDIALAB_MEDIAENGINE_API static std::unique_ptr<PcmPlaybackSession> create(std::shared_ptr<const MediaDocument>);
 RETROMEDIALAB_MEDIAENGINE_API ~PcmPlaybackSession();
 PcmPlaybackSession(const PcmPlaybackSession&)=delete;
 PcmPlaybackSession& operator=(const PcmPlaybackSession&)=delete;
 RETROMEDIALAB_MEDIAENGINE_API int channels()const noexcept;
 RETROMEDIALAB_MEDIAENGINE_API int sampleRate()const noexcept;
 RETROMEDIALAB_MEDIAENGINE_API std::int64_t framePosition()const noexcept;
 RETROMEDIALAB_MEDIAENGINE_API std::int64_t frameCount()const noexcept;
 RETROMEDIALAB_MEDIAENGINE_API std::size_t readSamples(std::span<std::int16_t>,std::stop_token={});
 RETROMEDIALAB_MEDIAENGINE_API void seekFrame(std::int64_t);
 // Idempotent; all held sources are released even if resetting the drive throws.
 RETROMEDIALAB_MEDIAENGINE_API void close();
private:
 struct State;explicit PcmPlaybackSession(std::unique_ptr<State>);
 std::unique_ptr<State> state_;
};
}
