#pragma once
#include "../Decoder.h"
namespace RetroMediaLab::MediaEngine {
class AtariTapeDecoder final:public SequentialDecoder {
public:
    RETROMEDIALAB_MEDIAENGINE_API std::string_view id()const noexcept override;
    RETROMEDIALAB_MEDIAENGINE_API std::span<const std::string> formatIds()const override;
    RETROMEDIALAB_MEDIAENGINE_API std::span<const std::string> machineIds()const override;
    RETROMEDIALAB_MEDIAENGINE_API bool canDecode(const MediaDocument&,std::optional<std::string> machine={})const override;
    RETROMEDIALAB_MEDIAENGINE_API std::future<SequentialDecodeResult> decodeAsync(const MediaDocument&,std::optional<std::string> machine={},std::stop_token={})const override;
};
namespace AtariTape {
struct Record {std::uint8_t type;std::span<const std::uint8_t> data;bool checksumValid;};
std::uint8_t checksum(std::span<const std::uint8_t>);
std::optional<Record> standardRecord(std::span<const std::uint8_t>);
bool extendedPadding(std::span<const std::uint8_t>);
}
}
