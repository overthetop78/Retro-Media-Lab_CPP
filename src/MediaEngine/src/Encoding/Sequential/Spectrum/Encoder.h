#pragma once
#include "../Encoder.h"
namespace RetroMediaLab::MediaEngine {
class SpectrumTapeEncoder final:public SequentialEncoder {
public:
    RETROMEDIALAB_MEDIAENGINE_API std::string_view id()const noexcept override;
    RETROMEDIALAB_MEDIAENGINE_API std::span<const std::string> formatIds()const override;
    RETROMEDIALAB_MEDIAENGINE_API std::span<const std::string> machineIds()const override;
    RETROMEDIALAB_MEDIAENGINE_API bool canEncode(const SequentialEncodeRequest&)const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::shared_ptr<const SequentialImage>> encodeAsync(SequentialEncodeRequest,std::stop_token={})const override;
};
}
