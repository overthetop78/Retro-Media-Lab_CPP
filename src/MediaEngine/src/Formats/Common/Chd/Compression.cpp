#include "Compression.h"
#include "CodecConstants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <zlib.h>
#include <lzma.h>
#include <zstd.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Chd {
struct Compression::Impl {
    z_stream inflater{};lzma_stream lzma=LZMA_STREAM_INIT;ZSTD_DCtx* zstd=nullptr;
    ~Impl(){if(inflater.state)inflateEnd(&inflater);lzma_end(&lzma);ZSTD_freeDCtx(zstd);}
};
Compression::Compression():impl_(std::make_unique<Impl>()){}
Compression::~Compression()=default;
void Compression::deflate(std::span<const std::uint8_t> source,std::span<std::uint8_t> destination,std::stop_token token){
    using namespace CodecConstants;checkCancellation(token);if(source.size()>MaximumHunk||destination.size()>MaximumHunk)throw MediaError(DiagnosticCode::ChdDeflate);auto& stream=impl_->inflater;
    if(!stream.state){if(inflateInit2(&stream,-MAX_WBITS)!=Z_OK)throw MediaError(DiagnosticCode::ChdDeflate);}else if(inflateReset(&stream)!=Z_OK)throw MediaError(DiagnosticCode::ChdDeflate);
    stream.next_in=const_cast<Bytef*>(source.data());stream.avail_in=uInt(source.size());std::size_t position=0;std::uint8_t extra{};
    for(;;){checkCancellation(token);const auto left=destination.size()-position,chunk=left?std::min(left,TransferChunk):std::size_t(1);stream.next_out=left?destination.data()+position:&extra;stream.avail_out=uInt(chunk);const auto before=stream.avail_in;const auto result=inflate(&stream,Z_FINISH);const auto produced=chunk-stream.avail_out;if(produced>left)throw MediaError(DiagnosticCode::ChdDeflate);position+=produced;
        if(result==Z_STREAM_END){if(position!=destination.size())throw MediaError(DiagnosticCode::ChdDeflate);return;}if((result!=Z_OK&&result!=Z_BUF_ERROR)||(before==stream.avail_in&&!produced))throw MediaError(DiagnosticCode::ChdDeflate);
    }
}
void Compression::lzma(std::span<const std::uint8_t> source,std::span<std::uint8_t> destination,std::stop_token token){
    using namespace CodecConstants;checkCancellation(token);if(source.size()>MaximumHunk||destination.size()>MaximumHunk)throw MediaError(DiagnosticCode::ChdLzma);auto dictionary=std::uint32_t(std::max(std::size_t(MinimumDictionary),std::min(std::size_t(MaximumDictionary),destination.size())));
    if(dictionary<DictionaryRoundingThreshold){auto rounded=MinimumDictionary;for(int bits=DictionaryFirstBits;rounded<dictionary;++bits){rounded=std::uint32_t(DictionaryEven)<<bits;if(rounded<dictionary)rounded=std::uint32_t(DictionaryOdd)<<bits;}dictionary=rounded;}else dictionary=(dictionary+DictionaryMiB-1)&~(DictionaryMiB-1);
    lzma_options_lzma options{};options.dict_size=dictionary;options.lc=LzmaLc;options.lp=LzmaLp;options.pb=LzmaPb;options.ext_flags=LZMA_LZMA1EXT_ALLOW_EOPM;lzma_set_ext_size(options,destination.size());const lzma_filter filters[]{{LZMA_FILTER_LZMA1EXT,&options},{LZMA_VLI_UNKNOWN,nullptr}};auto& stream=impl_->lzma;
    if(lzma_raw_decoder(&stream,filters)!=LZMA_OK)throw MediaError(DiagnosticCode::ChdLzma);stream.next_in=source.data();stream.avail_in=source.size();std::size_t position=0;std::uint8_t extra{};
    for(;;){checkCancellation(token);const auto left=destination.size()-position,chunk=left?std::min(left,TransferChunk):std::size_t(1);stream.next_out=left?destination.data()+position:&extra;stream.avail_out=chunk;const auto before=stream.avail_in;const auto result=lzma_code(&stream,LZMA_FINISH);const auto produced=chunk-stream.avail_out;if(produced>left)throw MediaError(DiagnosticCode::ChdLzma);position+=produced;
        if(result==LZMA_STREAM_END){if(position!=destination.size()||stream.avail_in)throw MediaError(DiagnosticCode::ChdLzma);return;}if(result!=LZMA_OK||(before==stream.avail_in&&!produced))throw MediaError(DiagnosticCode::ChdLzma);
    }
}
void Compression::zstd(std::span<const std::uint8_t> source,std::span<std::uint8_t> destination,std::stop_token token){
    using namespace CodecConstants;checkCancellation(token);if(source.size()>MaximumHunk||destination.size()>MaximumHunk)throw MediaError(DiagnosticCode::ChdZstd);if(!impl_->zstd)impl_->zstd=ZSTD_createDCtx();auto* stream=impl_->zstd;if(!stream||ZSTD_isError(ZSTD_DCtx_reset(stream,ZSTD_reset_session_only))||ZSTD_isError(ZSTD_DCtx_setParameter(stream,ZSTD_d_windowLogMax,ZstdMaximumWindowLog)))throw MediaError(DiagnosticCode::ChdZstd);
    ZSTD_inBuffer input{source.data(),source.size(),0};std::size_t position=0,last=0;std::uint8_t extra{};
    while(input.pos<input.size||last){checkCancellation(token);if(!last&&ZSTD_getDictID_fromFrame(source.data()+input.pos,source.size()-input.pos))throw MediaError(DiagnosticCode::ChdZstd);const auto left=destination.size()-position;ZSTD_outBuffer output{left?destination.data()+position:&extra,left?std::min(left,TransferChunk):std::size_t(1),0};const auto before=input.pos;last=ZSTD_decompressStream(stream,&output,&input);if(ZSTD_isError(last)||output.pos>left||(input.pos==before&&!output.pos))throw MediaError(DiagnosticCode::ChdZstd);position+=output.pos;}
    if(position!=destination.size())throw MediaError(DiagnosticCode::ChdZstd);
}
}
