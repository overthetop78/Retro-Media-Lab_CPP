#pragma once
#include "Constants.h"
#include "../../Encoding/Track.h"
#include "../../Decoding/Decoder.h"
#include <RetroMediaLab/MediaFileSystems/Sources.h>
namespace RetroMediaLab::MediaEngine::HpMmfm {
struct Header {std::uint8_t cylinder,head,sector;bool crcValid;};
struct Data {std::vector<std::uint8_t> payload;bool crcValid;};
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> encodePayload(MediaFileSystems::Bytes,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::vector<std::uint8_t> decodePayload(MediaFileSystems::Bytes,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::optional<std::vector<std::uint8_t>> decodeBytes(Flux::BitView,std::int64_t offset,std::int64_t count,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::optional<Header> decodeHeader(Flux::BitView,std::int64_t offset,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::int64_t findDataSync(Flux::BitView,std::int64_t headerOffset,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::optional<Data> decodeData(Flux::BitView,std::int64_t dataOffset,std::stop_token = {});
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
 Decoding::Result decode(const Flux::Revolution& revolution)const override{return HpMmfm::decode(revolution);}
};
}
