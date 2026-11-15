#include "Mfm.h"
#include "../../Models/Flux/BitReader.h"
#include "../../Decoding/Confidence.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::AmigaMfm {
namespace {
Decoding::LocalizedText state(std::optional<bool> valid){return Decoding::message(valid?(*valid?Constants::ValidKey:Constants::InvalidKey):Constants::UnavailableKey);}
struct Header {std::uint8_t cylinder,head,number;bool valid;std::uint64_t length;};
struct Data {std::vector<std::uint8_t> payload;bool valid;};
}
Decoding::Result decode(const Flux::Revolution& revolution,std::stop_token token){
    using namespace Constants;checkCancellation(token);const auto stream=Flux::decodeAdaptiveMfm(revolution.intervals,token);const auto bits=stream.bits();
    Decoding::Result result;result.decoderId=Id;result.displayName=DisplayName;result.estimatedCellTicks=stream.cellTicks();
    for(std::uint64_t offset=0;offset+SyncBitCount<=bits.size();++offset){
        checkCancellation(token);
        if(!Flux::match(bits,std::int64_t(offset),SyncWord)||!Flux::match(bits,std::int64_t(offset+EncodedByteBitCount),SyncWord))continue;
        const auto encoded=Flux::mfmBytes(bits,std::int64_t(offset+SyncBitCount),EncodedSectorByteCount,token);
        auto available=encoded?encoded:Flux::mfmBytes(bits,std::int64_t(offset+SyncBitCount),EncodedHeaderByteCount,token);
        std::optional<Header> header;std::optional<Data> data;
        if(available){
            const auto info=decodeOddEven(MediaFileSystems::Bytes(*available).first(InfoByteCount),token);
            const auto checksum=parity(*available,0,HeaderParitySourceByteCount,token);
            const bool valid=info[FormatByteOffset]==FormatByte&&(*available)[HeaderParityHighOffset]==checksum.first&&(*available)[HeaderParityLowOffset]==checksum.second;
            header=Header{std::uint8_t(info[TrackAndHeadOffset]>>TrackCylinderShift),std::uint8_t(info[TrackAndHeadOffset]&TrackHeadMask),info[SectorNumberOffset],valid,SyncBitCount+available->size()*EncodedByteBitCount};
            result.decodedBytes.insert(result.decodedBytes.end(),info.begin(),info.end());
        }
        if(encoded){
            const auto checksum=splitParity(*encoded,EncodedDataOffset,EncodedDataByteCount,token);
            const bool valid=(*encoded)[DataParityHighOffset]==checksum.first&&(*encoded)[DataParityLowOffset]==checksum.second;
            auto payload=decodeOddEven(MediaFileSystems::Bytes(*encoded).subspan(EncodedDataOffset,EncodedDataByteCount),token);
            result.decodedBytes.insert(result.decodedBytes.end(),payload.begin(),payload.end());data=Data{std::move(payload),valid};
        }
        const auto length=data?std::uint64_t(SyncBitCount+EncodedSectorByteCount*EncodedByteBitCount):header?header->length:std::uint64_t(SyncBitCount);
        Decoding::Sector sector;sector.cylinder=header?header->cylinder:0;sector.head=header?header->head:0;sector.number=header?header->number:0;
        sector.sizeCode=SectorSizeCode;sector.sizeBytes=SectorByteCount;sector.bitOffset=offset;sector.integrityKind=Decoding::IntegrityKind::Checksum;
        if((header&&!header->valid)||(data&&!data->valid))sector.integrityValid=false;else if(data)sector.integrityValid=true;
        if(data)sector.data=std::move(data->payload);
        using Decoding::literal;
        result.structures.push_back({Decoding::StructureKind::AmigaSync,offset,length,Decoding::message(StructureKey,{literal(std::to_string(sector.cylinder)),literal(std::to_string(sector.head)),literal(std::to_string(sector.number)),literal(std::to_string(SectorByteCount)),state(header?std::optional(header->valid):std::nullopt),state(data?std::optional(data->valid):std::nullopt)})});
        result.sectors.push_back(std::move(sector));offset+=length-1;
    }
    result.confidence=Decoding::confidence(std::int32_t(result.sectors.size()),std::int32_t(result.structures.size()),ConfidenceSectorWeight,ConfidenceDivisor);return result;
}
}
