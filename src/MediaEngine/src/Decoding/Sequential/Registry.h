#pragma once
#include "Decoder.h"
namespace RetroMediaLab::MediaEngine {
class SequentialDecoderRegistry final {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit SequentialDecoderRegistry(std::vector<std::shared_ptr<const SequentialDecoder>>);
    RETROMEDIALAB_MEDIAENGINE_API std::vector<std::shared_ptr<const SequentialDecoder>> findCompatible(const MediaDocument&,std::optional<std::string> machineId={},std::stop_token={})const;
    RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const SequentialDecoder> getRequired(const MediaDocument&,std::optional<std::string> machineId={},std::stop_token={})const;
    RETROMEDIALAB_MEDIAENGINE_API std::future<SequentialDecodeResult> decodeAsync(std::shared_ptr<const MediaDocument>,std::optional<std::string> machineId={},std::stop_token={})const;
private:std::vector<std::shared_ptr<const SequentialDecoder>> decoders_;
};
}
