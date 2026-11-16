#include "Container.h"
#include "EccTables.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Chd {
using namespace Constants;
namespace {
std::uint32_t normalizedCodec(std::uint32_t codec,std::uint32_t version){return version<Version5&&(codec==1||codec==MaximumLegacyCodec)?Zlib:codec;}
void computeEcc(std::span<const std::uint8_t> sector,std::span<const std::uint16_t> row,std::uint8_t& first,std::uint8_t& second){
    first=second=0;for(auto offset:row){const auto value=sector[CdModeOffset]==Mode2&&offset<CdAddressBytes?std::uint8_t(0):sector[CdDataOffset+offset];first^=value;second^=value;first=EccTables::ecclow[first];}
    first=EccTables::ecchigh[EccTables::ecclow[first]^second];second^=first;
}
void generateEcc(std::span<std::uint8_t> sector){
    for(int i=0;i<ParityPCount;++i)computeEcc(sector,std::span(EccTables::poffsets).subspan(std::size_t(i)*ParityPRow,ParityPRow),sector[ParityPOffset+i],sector[ParityPOffset+ParityPCount+i]);
    for(int i=0;i<ParityQCount;++i)computeEcc(sector,std::span(EccTables::qoffsets).subspan(std::size_t(i)*ParityQRow,ParityQRow),sector[ParityQOffset+i],sector[ParityQOffset+ParityQCount+i]);
}
}
void Container::initializeCodecs(){
    for(auto original:codecs_){const auto codec=normalizedCodec(original,version_);switch(codec){
        case 0:case Zlib:case Lzma:case Zstd:break;
        case HuffmanCodec:if(!huffman_)huffman_=std::make_unique<Huffman>(HuffmanSymbols,HuffmanMaximumBits);break;
        case FlacCodec:if(hunkBytes_%FlacAlignment)throw MediaError(DiagnosticCode::ChdCodec);break;
        case CdZlib:case CdLzma:case CdFlac:case CdZstd:if(hunkBytes_%CdBytes)throw MediaError(DiagnosticCode::ChdCodec);if(cdBuffer_.empty())cdBuffer_.resize(hunkBytes_);break;
        default:throw MediaError(DiagnosticCode::ChdCodec);}
    }
}
void Container::decompress(std::uint32_t original,std::span<const std::uint8_t> source,std::span<std::uint8_t> output,std::stop_token token){
    const auto codec=normalizedCodec(original,version_);
    switch(codec){
        case Zlib:codecsWorkspace_.deflate(source,output,token);return;
        case Lzma:codecsWorkspace_.lzma(source,output,token);return;
        case Zstd:codecsWorkspace_.zstd(source,output,token);return;
        case HuffmanCodec:huffman_->decompress(source,output,token);return;
        case FlacCodec:if(source.empty()||(source[0]!=LittleEndian&&source[0]!=BigEndian))throw MediaError(DiagnosticCode::ChdFlacOrder);flac_.decodeFrames(source.subspan(1),output,source[0]==BigEndian,token);return;
        case CdFlac:{const auto base=output.size()/CdBytes*CdMainBytes;const auto consumed=flac_.decodeFrames(source,std::span(cdBuffer_).first(base),true,token);codecsWorkspace_.deflate(source.subspan(consumed),std::span(cdBuffer_).subspan(base),token);reassembleCd(output,{},token);return;}
        case CdZlib:case CdLzma:case CdZstd:{const auto frames=output.size()/CdBytes,base=frames*CdMainBytes,eccBytes=(frames+EccByteMask)/EccByteBits,lengthBytes=std::size_t(output.size()<CdLengthBoundary?CdShortLength:CdLongLength),headerBytes=eccBytes+lengthBytes;
            if(source.size()<headerBytes)throw MediaError(DiagnosticCode::ChdCdHeader);std::size_t length=0;for(std::size_t i=0;i<lengthBytes;++i)length=(length<<ByteBits)|source[eccBytes+i];if(length>source.size()-headerBytes)throw MediaError(DiagnosticCode::ChdCdHeader);
            auto mainOut=std::span(cdBuffer_).first(base),subOut=std::span(cdBuffer_).subspan(base);auto main=source.subspan(headerBytes,length),sub=source.subspan(headerBytes+length);
            if(codec==CdZstd){codecsWorkspace_.zstd(main,mainOut,token);codecsWorkspace_.zstd(sub,subOut,token);}else{if(codec==CdLzma)codecsWorkspace_.lzma(main,mainOut,token);else codecsWorkspace_.deflate(main,mainOut,token);codecsWorkspace_.deflate(sub,subOut,token);}
            reassembleCd(output,source.first(eccBytes),token);return;}
        default:throw MediaError(DiagnosticCode::ChdCodec);
    }
}
void Container::reassembleCd(std::span<std::uint8_t> output,std::span<const std::uint8_t> ecc,std::stop_token token){
    const auto frames=output.size()/CdBytes;for(std::size_t frame=0;frame<frames;++frame){checkCancellation(token);auto sector=output.subspan(frame*CdBytes,CdBytes);std::copy_n(cdBuffer_.begin()+frame*CdMainBytes,CdMainBytes,sector.begin());std::copy_n(cdBuffer_.begin()+frames*CdMainBytes+frame*CdSubBytes,CdSubBytes,sector.begin()+CdMainBytes);
        if(!ecc.empty()&&(ecc[frame>>EccByteShift]&(1u<<(frame&EccByteMask)))){std::copy(CdSync.begin(),CdSync.end(),sector.begin());generateEcc(sector);}}
}
}
