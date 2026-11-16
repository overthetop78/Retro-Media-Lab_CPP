#include "Jfd.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <zlib.h>
#include <algorithm>
#include <array>
namespace RetroMediaLab::MediaEngine::Jfd {
namespace {
struct Inflater {
    z_stream stream{};
    Inflater(){if(inflateInit2(&stream,Constants::GzipWindowBits)!=Z_OK)throw MediaError(DiagnosticCode::JfdCompression);}
    ~Inflater(){inflateEnd(&stream);}
    Inflater(const Inflater&)=delete;
    Inflater& operator=(const Inflater&)=delete;
};
struct Deflater {
    z_stream stream{};
    Deflater(){if(deflateInit2(&stream,Z_DEFAULT_COMPRESSION,Z_DEFLATED,Constants::GzipWindowBits,Constants::DeflateMemoryLevel,Z_DEFAULT_STRATEGY)!=Z_OK)throw MediaError(DiagnosticCode::JfdCompressionWrite);}
    ~Deflater(){deflateEnd(&stream);}
    Deflater(const Deflater&)=delete;
    Deflater& operator=(const Deflater&)=delete;
};
}
std::vector<std::uint8_t> unpack(std::span<const std::uint8_t> bytes,std::stop_token token) {
    using namespace Constants;checkCancellation(token);
    if(bytes.size()>MaximumLength)throw MediaError(DiagnosticCode::JfdLength);
    if(bytes.size()<GzipSignature.size()||!std::equal(GzipSignature.begin(),GzipSignature.end(),bytes.begin()))return {bytes.begin(),bytes.end()};
    Inflater owner;auto& stream=owner.stream;
    stream.next_in=const_cast<Bytef*>(bytes.data());stream.avail_in=static_cast<uInt>(bytes.size());
    std::array<std::uint8_t,CompressionChunk> chunk{};std::vector<std::uint8_t> result;
    for(;;){checkCancellation(token);stream.next_out=chunk.data();stream.avail_out=static_cast<uInt>(chunk.size());
        const auto status=inflate(&stream,Z_NO_FLUSH);const auto size=chunk.size()-stream.avail_out;
        if(size>MaximumLength-result.size())throw MediaError(DiagnosticCode::JfdExpansion);
        result.insert(result.end(),chunk.begin(),chunk.begin()+size);
        if(status==Z_STREAM_END){
            if(stream.avail_in<GzipSignature.size()||!std::equal(GzipSignature.begin(),GzipSignature.end(),stream.next_in))break;
            if(inflateReset2(&stream,GzipWindowBits)!=Z_OK)throw MediaError(DiagnosticCode::JfdCompression);
        }else if(status!=Z_OK)throw MediaError(DiagnosticCode::JfdCompression);
    }return result;
}
void writeGzip(std::span<const std::uint8_t> bytes,const ImageOutput& output,std::stop_token token) {
    using namespace Constants;checkCancellation(token);Deflater owner;auto& stream=owner.stream;
    std::array<std::uint8_t,CompressionChunk> chunk{};std::size_t offset{};
    for(;;){checkCancellation(token);
        if(stream.avail_in==0){const auto count=std::min(CompressionChunk,bytes.size()-offset);stream.next_in=count?const_cast<Bytef*>(bytes.data()+offset):nullptr;stream.avail_in=static_cast<uInt>(count);offset+=count;}
        stream.next_out=chunk.data();stream.avail_out=static_cast<uInt>(chunk.size());
        const auto status=deflate(&stream,offset==bytes.size()?Z_FINISH:Z_NO_FLUSH);
        if(status!=Z_OK&&status!=Z_STREAM_END)throw MediaError(DiagnosticCode::JfdCompressionWrite);
        const auto count=chunk.size()-stream.avail_out;if(count!=0)output({chunk.data(),count});
        if(status==Z_STREAM_END)break;
    }
}
}
