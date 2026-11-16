#pragma once
#include "Bitstream.h"
#include <vector>
namespace RetroMediaLab::MediaEngine::Chd {
class Huffman final {
public:
    RETROMEDIALAB_MEDIAENGINE_API Huffman(int symbols,int maximumBits);
    RETROMEDIALAB_MEDIAENGINE_API int decode(Bitstream&)const;
    RETROMEDIALAB_MEDIAENGINE_API void importRle(Bitstream&);
    RETROMEDIALAB_MEDIAENGINE_API void importHuffman(Bitstream&);
    RETROMEDIALAB_MEDIAENGINE_API void decompress(std::span<const std::uint8_t>,std::span<std::uint8_t>,std::stop_token={});
private:
    void build();int maximumBits_;std::vector<std::uint8_t> lengths_;std::vector<std::uint32_t> lookup_;
};
}
