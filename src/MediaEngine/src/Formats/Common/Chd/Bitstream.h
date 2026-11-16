#pragma once
#include "CodecConstants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <span>
namespace RetroMediaLab::MediaEngine::Chd {
// The input is borrowed for this decoder's lifetime. Read-ahead is zero-padded;
// overflow accounts only for the bits actually consumed, as in the C# reader.
class Bitstream final {
public:
    explicit Bitstream(std::span<const std::uint8_t> source):source_(source){}
    bool overflow()const noexcept{return offset_-std::uint64_t(bits_/CodecConstants::ByteBits)>source_.size();}
    std::uint64_t offset()const noexcept{return offset_-std::uint64_t(bits_/CodecConstants::ByteBits);}
    std::uint32_t peek(int count){using namespace CodecConstants;if(count<0||count>WordBits)throw MediaError(DiagnosticCode::ChdBitWidth);if(!count)return 0;while(bits_<count){buffer_=(buffer_<<ByteBits)|(offset_<source_.size()?source_[std::size_t(offset_)]:0);++offset_;bits_+=ByteBits;}return std::uint32_t((buffer_>>(bits_-count))&((std::uint64_t(1)<<count)-1));}
    void remove(int count){bits_-=count;buffer_=bits_?buffer_&((std::uint64_t(1)<<bits_)-1):0;}
    std::uint32_t read(int count){const auto value=peek(count);remove(count);return value;}
    std::uint64_t flush(){while(bits_>=CodecConstants::ByteBits){--offset_;bits_-=CodecConstants::ByteBits;}bits_=0;buffer_=0;return offset_;}
private:std::span<const std::uint8_t> source_;std::uint64_t buffer_=0;int bits_=0;std::uint64_t offset_=0;
};
}
