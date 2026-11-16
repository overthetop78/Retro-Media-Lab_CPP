#include "Compression.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <zlib.h>
#include <array>
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaEngine::Csw {
namespace {
struct Inflater {
    z_stream stream{};
    Inflater(){if(inflateInit(&stream)!=Z_OK)throw MediaError(DiagnosticCode::CswCompression);}
    ~Inflater(){inflateEnd(&stream);}
    Inflater(const Inflater&)=delete;Inflater& operator=(const Inflater&)=delete;
};
struct Deflater {
    z_stream stream{};
    Deflater(){if(deflateInit(&stream,Z_DEFAULT_COMPRESSION)!=Z_OK)throw MediaError(DiagnosticCode::CswDeflate);}
    ~Deflater(){deflateEnd(&stream);}
    Deflater(const Deflater&)=delete;Deflater& operator=(const Deflater&)=delete;
};
}
void inflatePulses(std::span<const std::uint8_t> bytes,const ImageOutput& output,std::stop_token token){
    if(bytes.size()>std::numeric_limits<uInt>::max())throw MediaError(DiagnosticCode::CswCompression);
    Inflater owner;auto& stream=owner.stream;stream.next_in=const_cast<Bytef*>(bytes.data());stream.avail_in=static_cast<uInt>(bytes.size());
    std::array<std::uint8_t,Constants::CompressionBuffer> chunk{};
    for(;;){checkCancellation(token);stream.next_out=chunk.data();stream.avail_out=static_cast<uInt>(chunk.size());const auto status=inflate(&stream,Z_NO_FLUSH);const auto count=chunk.size()-stream.avail_out;if(count)output({chunk.data(),count});if(status==Z_STREAM_END)break;if(status!=Z_OK)throw MediaError(DiagnosticCode::CswCompression);}
}
void deflatePulses(std::span<const std::uint8_t> bytes,const ImageOutput& output,std::stop_token token){
    Deflater owner;auto& stream=owner.stream;std::array<std::uint8_t,Constants::CompressionBuffer> chunk{};std::size_t offset{};
    for(;;){checkCancellation(token);if(!stream.avail_in){const auto count=std::min(chunk.size(),bytes.size()-offset);stream.next_in=count?const_cast<Bytef*>(bytes.data()+offset):nullptr;stream.avail_in=static_cast<uInt>(count);offset+=count;}stream.next_out=chunk.data();stream.avail_out=static_cast<uInt>(chunk.size());const auto status=deflate(&stream,offset==bytes.size()?Z_FINISH:Z_NO_FLUSH);if(status!=Z_OK&&status!=Z_STREAM_END)throw MediaError(DiagnosticCode::CswDeflate);const auto count=chunk.size()-stream.avail_out;if(count)output({chunk.data(),count});if(status==Z_STREAM_END)break;}
}
}
