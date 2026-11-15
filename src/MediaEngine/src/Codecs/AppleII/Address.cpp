#include "Codec.h"
#include "Constants.h"
namespace RetroMediaLab::MediaEngine::AppleII {
std::pair<std::uint8_t,std::uint8_t> encodeFourAndFour(std::uint8_t value) noexcept{return {std::uint8_t((value>>1)|Constants::FourAndFourMask),std::uint8_t(value|Constants::FourAndFourMask)};}
std::array<std::uint8_t,Constants::EncodedAddressByteCount> encodeAddress(std::uint8_t volume,std::uint8_t cylinder,std::uint8_t sector) noexcept {
    const std::array values{volume,cylinder,sector,addressChecksum(volume,cylinder,sector)};std::array<std::uint8_t,Constants::EncodedAddressByteCount> encoded{};
    for(std::size_t index=0;index<values.size();++index){const auto pair=encodeFourAndFour(values[index]);encoded[index*Constants::EncodedBytesPerAddressValue]=pair.first;encoded[index*Constants::EncodedBytesPerAddressValue+1]=pair.second;}
    return encoded;
}
}
