#include "Internal.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <lz4.h>
#include <zlib.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::HxcStream {
void appendLittleEndian(std::vector<std::uint8_t>& bytes,std::uint32_t value,std::size_t width) {
    for(std::size_t index=0;index<width;++index)bytes.push_back(std::uint8_t(value>>(index*Constants::ByteBits)));
}
std::uint32_t checksum(std::span<const std::uint8_t> bytes,std::uint32_t seed) {
    auto crc=static_cast<uLong>(seed);
    for(std::size_t at=0;at<bytes.size();){const auto count=std::min(Constants::CrcChunkBytes,bytes.size()-at);crc=::crc32(crc,bytes.data()+at,static_cast<uInt>(count));at+=count;}
    return std::uint32_t(crc);
}
void appendPulse(std::vector<std::uint8_t>& bytes,std::uint32_t pulse) {
    using namespace Constants;
    if(pulse<PulseOneLimit)bytes.push_back(std::uint8_t(pulse));
    else if(pulse<PulseTwoLimit){bytes.push_back(std::uint8_t(PulseTwoPrefix|(pulse>>ByteBits)));bytes.push_back(std::uint8_t(pulse));}
    else if(pulse<PulseThreeLimit){bytes.push_back(std::uint8_t(PulseThreePrefix|(pulse>>TwoByteShift)));bytes.push_back(std::uint8_t(pulse>>ByteBits));bytes.push_back(std::uint8_t(pulse));}
    else{bytes.push_back(std::uint8_t(PulseFourPrefix|(pulse>>ThreeByteShift)));bytes.push_back(std::uint8_t(pulse>>TwoByteShift));bytes.push_back(std::uint8_t(pulse>>ByteBits));bytes.push_back(std::uint8_t(pulse));}
}
void appendBlock(std::vector<std::uint8_t>& bytes,std::uint32_t type,std::span<const std::uint8_t> data,std::optional<std::uint32_t> count) {
    using namespace Constants;std::vector<std::uint8_t> packed;auto content=data;
    if(type!=MetadataBlock){
        if(data.size()>MaximumSignedSize)throw MediaError(DiagnosticCode::HxcLz4Encode);
        const auto bound=LZ4_compressBound(static_cast<int>(data.size()));if(bound<=0)throw MediaError(DiagnosticCode::HxcLz4Encode);packed.resize(std::size_t(bound));
        const auto length=LZ4_compress_default(reinterpret_cast<const char*>(data.data()),reinterpret_cast<char*>(packed.data()),static_cast<int>(data.size()),bound);
        if(length<=0)throw MediaError(DiagnosticCode::HxcLz4Encode);packed.resize(std::size_t(length));content=packed;
    }
    const auto extra=type==MetadataBlock?0:count?StreamExtraBytes:IoExtraBytes;
    if(bytes.size()>MaximumChunkBytes||content.size()+extra+BlockHeaderBytes>MaximumChunkBytes-bytes.size())throw MediaError(DiagnosticCode::HxcChunkLength);
    appendLittleEndian(bytes,type);appendLittleEndian(bytes,std::uint32_t(extra+content.size()));
    if(extra!=0){appendLittleEndian(bytes,std::uint32_t(content.size()));appendLittleEndian(bytes,std::uint32_t(data.size()));}
    if(count)appendLittleEndian(bytes,*count);bytes.insert(bytes.end(),content.begin(),content.end());
    while(bytes.size()%Alignment!=0)bytes.push_back(0);
}
}
