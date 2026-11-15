#include "Mmfm.h"
#include "../../Primitives/Bits.h"
#include "../../Models/Flux/BitReader.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::HpMmfm {
std::vector<std::uint8_t> encodePayload(MediaFileSystems::Bytes payload,std::stop_token token){
 checkCancellation(token);if(payload.size()%Constants::PairByteCount)throw MediaError(DiagnosticCode::HpPairCount,{std::to_string(payload.size())});std::vector<std::uint8_t> result(payload.size());
 for(std::size_t index=0;index<payload.size();index+=Constants::PairByteCount){checkCancellation(token);result[index]=Primitives::reverseBits(payload[index+1]);result[index+1]=Primitives::reverseBits(payload[index]);}return result;
}
std::vector<std::uint8_t> decodePayload(MediaFileSystems::Bytes payload,std::stop_token token){return encodePayload(payload,token);}
std::optional<std::vector<std::uint8_t>> decodeBytes(Flux::BitView bits,std::int64_t offset,std::int64_t count,std::stop_token token){return Flux::mfmBytes(bits,offset,count,token);}
}
