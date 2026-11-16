#pragma once
#include "Encoder.h"
namespace RetroMediaLab::MediaEngine {
class SequentialEncoderRegistry final {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit SequentialEncoderRegistry(std::vector<std::shared_ptr<const SequentialEncoder>>);
    RETROMEDIALAB_MEDIAENGINE_API std::vector<std::shared_ptr<const SequentialEncoder>> findCompatible(const SequentialEncodeRequest&,std::stop_token={})const;
    RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const SequentialEncoder> getRequired(const SequentialEncodeRequest&,std::stop_token={})const;
    RETROMEDIALAB_MEDIAENGINE_API std::future<std::shared_ptr<const SequentialImage>> encodeAsync(SequentialEncodeRequest,std::stop_token={})const;
private:
    static bool signalTarget(const SequentialEncodeRequest&);
    std::vector<std::shared_ptr<const SequentialEncoder>> encoders_;
};
}
