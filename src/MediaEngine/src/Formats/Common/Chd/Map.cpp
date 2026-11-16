#include "Container.h"
#include "../../../Codecs/Crc16.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Chd {
using namespace Constants;
void Container::readLegacyMap(std::uint64_t offset,std::stop_token token){
    const auto width=version_<Version3?OldEntryBytes:LegacyEntryBytes;const auto tableBytes=(std::uint64_t(hunkCount_)+1)*width;if(offset>source_->length()||tableBytes>source_->length()-offset)throw MediaError(DiagnosticCode::ChdRange);legacyMap_.resize(hunkCount_);std::array<std::uint8_t,LegacyEntryBytes> bytes{};
    for(auto& entry:legacyMap_){readAt(offset,std::span(bytes).first(width),token);const auto block=qFromBigEndian<std::uint64_t>(bytes.data());
        if(version_<Version3){entry.offset=block&OldOffsetMask;entry.length=std::uint32_t(block>>OldLengthShift);entry.type=entry.length==hunkBytes_?LegacyRaw:LegacyCompressed;}
        else{entry.offset=block;entry.length=qFromBigEndian<std::uint16_t>(bytes.data()+LegacyLengthOffset)|(std::uint32_t(bytes[LegacyLengthHigh])<<WordBits);entry.type=bytes[LegacyTypeOffset]&LegacyTypeMask;}
        if((entry.type==LegacyCompressed||entry.type==LegacyRaw)&&(entry.offset>source_->length()||entry.length>source_->length()-entry.offset))throw MediaError(DiagnosticCode::ChdMap);offset+=width;
    }
    readAt(offset,std::span(bytes).first(width),token);if(!std::equal(bytes.begin(),bytes.begin()+width,Cookie.begin()))throw MediaError(DiagnosticCode::ChdMap);
}
void Container::readMap5(std::uint64_t offset,std::stop_token token){
    const auto mapLength=std::uint64_t(hunkCount_)*(codecs_[0]?MapEntryBytes:UncompressedMapBytes);if(mapLength>MaximumMap)throw MediaError(DiagnosticCode::ChdDimensions);if(!codecs_[0]&&(offset>source_->length()||mapLength>source_->length()-offset))throw MediaError(DiagnosticCode::ChdRange);map_.resize(std::size_t(mapLength));
    if(!codecs_[0]){readAt(offset,map_,token);return;}
    std::array<std::uint8_t,MapHeaderBytes> header{};readAt(offset,header,token);const auto length=qFromBigEndian<std::uint32_t>(header.data()+MapLengthOffset);
    const auto payload=offset+MapHeaderBytes;if(length>MaximumMap||payload>source_->length()||length>source_->length()-payload)throw MediaError(DiagnosticCode::ChdRange);
    auto current=(std::uint64_t(qFromBigEndian<std::uint16_t>(header.data()+MapDataOffset))<<CodecConstants::WordBits)|qFromBigEndian<std::uint32_t>(header.data()+MapDataOffset+sizeof(std::uint16_t));
    const auto expected=qFromBigEndian<std::uint16_t>(header.data()+MapCrcOffset);std::vector<std::uint8_t> compressed(length);readAt(payload,compressed,token);Bitstream bits(compressed);Huffman decoder(MapSymbols,MapCodeBits);decoder.importRle(bits);
    std::uint8_t last=0;int repeat=0;
    for(std::uint32_t i=0;i<hunkCount_;++i){if(!(i&PollMask))checkCancellation(token);std::uint8_t type;
        if(repeat){type=last;--repeat;}else{if(bits.overflow())throw MediaError(DiagnosticCode::ChdMap);const auto value=decoder.decode(bits);
            if(value==SmallRun){type=last;repeat=SmallRunBias+decoder.decode(bits);}else if(value==LargeRun){type=last;repeat=LargeRunBias+(decoder.decode(bits)<<RunShift)+decoder.decode(bits);}else type=last=std::uint8_t(value);}
        map_[std::size_t(i)*MapEntryBytes]=type;
    }
    std::uint32_t lastSelf=0;std::uint64_t lastParent=0;
    for(std::uint32_t i=0;i<hunkCount_;++i){if(!(i&PollMask))checkCancellation(token);auto* entry=map_.data()+std::size_t(i)*MapEntryBytes;auto blockOffset=current;std::uint32_t blockLength=0;std::uint16_t crc=0;
        switch(entry[0]){
            case 0:case 1:case CodecThird:case CodecLast:blockLength=bits.read(header[LengthBitsOffset]);current+=blockLength;crc=std::uint16_t(bits.read(WordBits));break;
            case RawType:blockLength=hunkBytes_;current+=blockLength;crc=std::uint16_t(bits.read(WordBits));break;
            case SelfType:blockOffset=lastSelf=bits.read(header[SelfBitsOffset]);break;
            case ParentType:blockOffset=lastParent=bits.read(header[ParentBitsOffset]);break;
            case SelfNext:++lastSelf;[[fallthrough]];
            case SelfLast:entry[0]=SelfType;blockOffset=lastSelf;break;
            case ParentCurrent:entry[0]=ParentType;blockOffset=lastParent=std::uint64_t(i)*hunkBytes_/unitBytes_;break;
            case ParentNext:lastParent+=hunkBytes_/unitBytes_;[[fallthrough]];
            case ParentLast:entry[0]=ParentType;blockOffset=lastParent;break;
            default:throw MediaError(DiagnosticCode::ChdMap);}
        entry[EntryLengthOffset]=std::uint8_t(blockLength>>WordBits);qToBigEndian(std::uint16_t(blockLength),entry+EntryLengthOffset+1);
        qToBigEndian(std::uint16_t(blockOffset>>CodecConstants::WordBits),entry+EntryDataOffset);qToBigEndian(std::uint32_t(blockOffset),entry+EntryDataOffset+sizeof(std::uint16_t));qToBigEndian(crc,entry+EntryCrcOffset);
    }
    if(bits.overflow())throw MediaError(DiagnosticCode::ChdMap);if(Crc16::compute(map_,Crc16::Constants::Ccitt,Crc16::Constants::AllBitsSet,token)!=expected)throw MediaError(DiagnosticCode::ChdMapCrc);
}
}
