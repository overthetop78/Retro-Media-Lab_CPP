#pragma once
#include "Request.h"
namespace RetroMediaLab::MediaEngine {
class SequentialEncoder {
public:
    virtual ~SequentialEncoder()=default;
    virtual std::string_view id()const noexcept=0;
    virtual std::span<const std::string> formatIds()const=0;
    virtual std::span<const std::string> machineIds()const=0;
    virtual bool canEncode(const SequentialEncodeRequest&)const=0;
    virtual std::future<std::shared_ptr<const SequentialImage>> encodeAsync(SequentialEncodeRequest,std::stop_token={})const=0;
};
}
