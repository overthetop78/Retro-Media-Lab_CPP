#include "Crc32C.h"
#include "Crc32CConstants.h"
#include <array>
namespace RetroMediaLab::MediaEngine::Primitives {
namespace {
constexpr auto table(){using namespace Crc32CConstants;std::array<std::uint32_t,TableSize> result{};for(std::size_t index=0;index<result.size();++index){auto value=std::uint32_t(index);for(int bit=0;bit<ByteBits;++bit)value=(value>>1)^((value&1)?Polynomial:0);result[index]=value;}return result;}
inline constexpr auto Table=table();
}
std::uint32_t crc32c(std::span<const std::uint8_t> bytes)noexcept{using namespace Crc32CConstants;auto value=Initial;for(auto byte:bytes)value=Table[(value^byte)&ByteMask]^(value>>ByteBits);return ~value;}
}
