#pragma once
#include "Models.h"
namespace RetroMediaLab::MediaEngine {
class SequentialDecoder {
public:
    virtual ~SequentialDecoder()=default;
    virtual std::string_view id()const noexcept=0;
    virtual std::span<const std::string> formatIds()const=0;
    virtual std::span<const std::string> machineIds()const=0;
    virtual bool canDecode(const MediaDocument&,std::optional<std::string> machineId={})const=0;
    // The caller retains the document until completion. Registry::decodeAsync owns it.
    virtual std::future<SequentialDecodeResult> decodeAsync(const MediaDocument&,std::optional<std::string> machineId={},std::stop_token={})const=0;
};
}
