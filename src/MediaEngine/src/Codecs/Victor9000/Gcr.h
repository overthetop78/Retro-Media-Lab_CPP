#pragma once
#include "Constants.h"
#include "../../Encoding/Track.h"
#include "../../Decoding/Decoder.h"
#include <RetroMediaLab/MediaFileSystems/Sources.h>
namespace RetroMediaLab::MediaEngine::Victor9000 {
struct Header {std::uint8_t cylinder,sector;std::vector<std::uint8_t> bytes;bool valid;};
struct Data {std::uint8_t prefix;std::vector<std::uint8_t> payload;std::uint16_t storedChecksum;bool valid;std::uint64_t endOffset;};
inline std::uint8_t headerChecksum(std::uint8_t cylinder,std::uint8_t sector)noexcept{return std::uint8_t(cylinder+sector);}
RETROMEDIALAB_MEDIAENGINE_API std::uint16_t checksum(MediaFileSystems::Bytes,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API void addBlock(Flux::Bits&,MediaFileSystems::Bytes marker,MediaFileSystems::Bytes values,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::optional<Header> decodeHeader(Flux::BitView,std::int64_t markOffset,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::optional<Data> decodeData(Flux::BitView,std::int64_t markOffset,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::int64_t findDataMark(Flux::BitView,std::int64_t start,std::int64_t end,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API void collectUnpairedData(Flux::BitView,std::span<const std::int64_t> paired,std::vector<Decoding::Structure>&,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API Decoding::Result decode(const Flux::Revolution&,std::stop_token = {});
class Encoder final:public Encoding::TrackEncoder {
public:
 std::string_view id()const noexcept override{return Constants::Id;}
 std::string_view displayName()const noexcept override{return Constants::DisplayName;}
protected:
 RETROMEDIALAB_MEDIAENGINE_API Flux::Bits encodeBits(const Encoding::Request&,std::stop_token)const override;
};
class Decoder final:public Decoding::Decoder {
public:
 std::string_view id()const noexcept override{return Constants::Id;}
 std::string_view displayName()const noexcept override{return Constants::DisplayName;}
 Decoding::Result decode(const Flux::Revolution& revolution)const override{return Victor9000::decode(revolution);}
};
}
