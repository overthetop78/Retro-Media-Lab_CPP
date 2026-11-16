#pragma once
#include <RetroMediaLab/MediaEngine/Export.h>
#include <vector>
#include <span>
#include <optional>
#include <string_view>
#include <cstdint>
namespace RetroMediaLab::MediaEngine::MfmEmu {
class Header final {
public:
    RETROMEDIALAB_MEDIAENGINE_API static std::optional<Header> decode(std::span<const std::uint8_t>);
    RETROMEDIALAB_MEDIAENGINE_API static Header create(std::int32_t cylinders,std::int32_t heads,std::int32_t trackBytes,std::uint32_t sampleRate,std::uint32_t start=0,std::string_view command={},std::string_view note={});
    std::uint32_t version()const noexcept{return version_;}std::uint32_t sampleRate()const noexcept{return rate_;}std::uint32_t startNanoseconds()const noexcept{return start_;}
    std::int32_t headerLength()const noexcept{return length_;}std::int32_t trackDataLength()const noexcept{return dataLength_;}std::int32_t trackHeaderLength()const noexcept{return trackHeader_;}std::int32_t cylinders()const noexcept{return cylinders_;}std::int32_t heads()const noexcept{return heads_;}
    const std::vector<std::uint8_t>& bytes()const noexcept{return bytes_;}
    std::span<const std::uint8_t> commandLine()const noexcept{return std::span(bytes_).subspan(commandOffset_,commandLength_);}std::span<const std::uint8_t> note()const noexcept{return std::span(bytes_).subspan(noteOffset_,noteLength_);}
    std::uint64_t trackRecordLength()const noexcept{return std::uint64_t(trackHeader_)+dataLength_;}
    std::uint64_t endMarkerOffset()const noexcept{return length_+std::uint64_t(cylinders_)*heads_*trackRecordLength();}
    RETROMEDIALAB_MEDIAENGINE_API std::uint64_t trackOffset(std::int32_t cylinder,std::int32_t head)const;
    RETROMEDIALAB_MEDIAENGINE_API bool trackAddress(std::span<const std::uint8_t>,std::int32_t& cylinder,std::int32_t& head,bool& end)const;
private:
    Header()=default;
    std::uint32_t version_{},rate_{},start_{};std::int32_t length_{},dataLength_{},trackHeader_{},cylinders_{},heads_{};std::vector<std::uint8_t> bytes_;std::size_t commandOffset_{},commandLength_{},noteOffset_{},noteLength_{};
};
}
