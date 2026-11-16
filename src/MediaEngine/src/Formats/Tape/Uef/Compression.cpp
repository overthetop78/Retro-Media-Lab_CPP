#include "Compression.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <zlib.h>
#include <array>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Uef {
namespace {
using namespace Constants;
struct Inflater {
    z_stream stream{};
    Inflater(){if(inflateInit2(&stream,GzipWindowBits)!=Z_OK)throw MediaError(DiagnosticCode::UefCompression);}
    ~Inflater(){inflateEnd(&stream);}
    Inflater(const Inflater&)=delete;Inflater& operator=(const Inflater&)=delete;
};
struct Deflater {
    z_stream stream{};
    Deflater(){if(deflateInit2(&stream,Z_DEFAULT_COMPRESSION,Z_DEFLATED,GzipWindowBits,DeflateMemoryLevel,Z_DEFAULT_STRATEGY)!=Z_OK)throw MediaError(DiagnosticCode::UefCompression);}
    ~Deflater(){deflateEnd(&stream);}
    Deflater(const Deflater&)=delete;Deflater& operator=(const Deflater&)=delete;
};
}
std::vector<std::uint8_t> logicalBytes(const RecognitionContext& context,bool& gzip,bool headerOnly,std::stop_token token){
    using namespace Constants;checkCancellation(token);const auto first=context.readHeaderAsync(WordBytes,token).get();gzip=first.size()>=WordBytes&&first[0]==GzipIdentificationFirst&&first[1]==GzipIdentificationSecond;
    if(!gzip){if(!headerOnly&&context.length()>MaximumDecompressedLength)throw MediaError(DiagnosticCode::UefSize);return headerOnly?context.readHeaderAsync(HeaderSize,token).get():context.readBytesAsync(token).get();}
    Inflater owner;auto& stream=owner.stream;std::vector<std::uint8_t> input,result;std::array<std::uint8_t,CompressionBuffer> chunk{};std::int64_t offset{};
    for(;;){checkCancellation(token);if(!stream.avail_in&&offset<context.length()){const auto count=static_cast<std::int32_t>(std::min<std::int64_t>(CompressionBuffer,context.length()-offset));input=context.readAsync(offset,count,token).get();if(input.empty())throw MediaError(DiagnosticCode::UefCompression);offset+=input.size();stream.next_in=input.data();stream.avail_in=static_cast<uInt>(input.size());}
        const auto capacity=headerOnly?HeaderSize-result.size():chunk.size();stream.next_out=chunk.data();stream.avail_out=static_cast<uInt>(capacity);const auto status=inflate(&stream,Z_NO_FLUSH);const auto count=capacity-stream.avail_out;if(count>MaximumDecompressedLength-result.size())throw MediaError(DiagnosticCode::UefSize);result.insert(result.end(),chunk.begin(),chunk.begin()+count);if(headerOnly&&result.size()==HeaderSize)return result;
        if(status==Z_STREAM_END){if(!stream.avail_in&&offset==context.length())return result;if(inflateReset2(&stream,GzipWindowBits)!=Z_OK)throw MediaError(DiagnosticCode::UefCompression);}else if(status!=Z_OK)throw MediaError(DiagnosticCode::UefCompression);
    }
}
void writeGzip(const std::function<void(const ImageOutput&)>& producer,const ImageOutput& output,std::stop_token token){
    using namespace Constants;Deflater owner;auto& stream=owner.stream;std::array<std::uint8_t,CompressionBuffer> chunk{};
    producer([&](std::span<const std::uint8_t> bytes){std::size_t offset{};while(offset<bytes.size()){checkCancellation(token);const auto size=std::min(chunk.size(),bytes.size()-offset);stream.next_in=const_cast<Bytef*>(bytes.data()+offset);stream.avail_in=static_cast<uInt>(size);offset+=size;while(stream.avail_in){checkCancellation(token);stream.next_out=chunk.data();stream.avail_out=static_cast<uInt>(chunk.size());if(deflate(&stream,Z_NO_FLUSH)!=Z_OK)throw MediaError(DiagnosticCode::UefCompression);const auto count=chunk.size()-stream.avail_out;if(count)output({chunk.data(),count});}}});
    for(;;){checkCancellation(token);stream.next_out=chunk.data();stream.avail_out=static_cast<uInt>(chunk.size());const auto status=deflate(&stream,Z_FINISH);if(status!=Z_OK&&status!=Z_STREAM_END)throw MediaError(DiagnosticCode::UefCompression);const auto count=chunk.size()-stream.avail_out;if(count)output({chunk.data(),count});if(status==Z_STREAM_END)break;}
}
}
