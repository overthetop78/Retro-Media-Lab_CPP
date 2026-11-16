#include "Internal.h"
#include <lz4.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::HxcStream {
namespace {
using namespace Constants;
std::uint32_t bounded(std::span<const std::uint8_t> bytes,std::size_t at,std::uint32_t maximum=MaximumUnpackedBytes){const auto value=readU32(bytes,at);if(value>maximum)throw MediaError(DiagnosticCode::HxcBlockSize,{std::to_string(value)});return value;}
std::vector<std::uint8_t> unpack(std::span<const std::uint8_t> packed,std::size_t size){std::vector<std::uint8_t> bytes(size);const auto count=LZ4_decompress_safe(reinterpret_cast<const char*>(packed.data()),reinterpret_cast<char*>(bytes.data()),static_cast<int>(packed.size()),static_cast<int>(size));if(count!=std::int64_t(size))throw MediaError(DiagnosticCode::HxcLz4Incomplete);return bytes;}
}
DecodedTrack readTrack(std::vector<std::uint8_t> data,std::stop_token token) {
    using namespace Constants;checkCancellation(token);DecodedTrack result;result.nativeBytes=std::move(data);const std::span<const std::uint8_t> bytes(result.nativeBytes);
    std::vector<std::uint32_t> flux;std::vector<std::uint16_t> io;auto resolution=DefaultResolution;std::size_t offset{};
    while(offset<bytes.size()){checkCancellation(token);requireBytes(bytes,offset,ChunkHeaderBytes+CrcBytes,bytes.size());const auto size=readU32(bytes,offset+ChunkSizeOffset);
        if(readU32(bytes,offset)!=ChunkSignature||size<ChunkHeaderBytes+CrcBytes||size>MaximumChunkBytes)throw MediaError(DiagnosticCode::HxcChunk,{std::to_string(offset)});
        requireBytes(bytes,offset,size,bytes.size());const auto end=offset+size-CrcBytes;
        if(checksum(bytes.subspan(offset,size-CrcBytes),CrcSeed)!=readU32(bytes,end))throw MediaError(DiagnosticCode::HxcCrc,{std::to_string(offset)});
        auto block=offset+ChunkHeaderBytes;
        while(block<end){checkCancellation(token);requireBytes(bytes,block,BlockHeaderBytes,end);const auto type=readU32(bytes,block),payload=readU32(bytes,block+FieldBytes);
            if(type==MetadataBlock){requireBytes(bytes,block+BlockHeaderBytes,payload,end);readMetadata(bytes.subspan(block+BlockHeaderBytes,payload),result.metadata,resolution);block+=BlockHeaderBytes+payload;}
            else if(type==PackedIoBlock||type==PackedStreamBlock){const auto header=type==PackedIoBlock?IoHeaderBytes:StreamHeaderBytes;requireBytes(bytes,block,header,end);
                const auto packed=bounded(bytes,block+PackedSizeOffset),unpacked=bounded(bytes,block+UnpackedSizeOffset);
                if(type==PackedIoBlock&&unpacked%WordBytes!=0)throw MediaError(DiagnosticCode::HxcIoOdd);
                const auto pulses=type==PackedStreamBlock?bounded(bytes,block+PulseCountOffset,MaximumPulseCount):0;
                requireBytes(bytes,block+header,packed,end);const auto decoded=unpack(bytes.subspan(block+header,packed),unpacked);
                if(type==PackedIoBlock)for(std::size_t at=0;at<decoded.size();at+=WordBytes)io.push_back(std::uint16_t(decoded[at]|std::uint32_t(decoded[at+1])<<ByteBits));
                else decodePulses(decoded,pulses,flux,token);
                block+=header+packed;
            }else throw MediaError(DiagnosticCode::HxcBlockType,{std::to_string(type),std::to_string(block)});
            block=(block+Alignment-1)&~(Alignment-1);
        }offset+=size;
    }
    result.revolutions=createRevolutions(flux,io,resolution,token);result.writeProtected=std::any_of(io.begin(),io.end(),[](auto value){return (value&WriteProtectedFlag)!=0;});return result;
}
}
