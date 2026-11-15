#pragma once
#include "Constants.h"
#include "../../Encoding/Track.h"
#include "../../Decoding/Decoder.h"
namespace RetroMediaLab::MediaEngine::Northstar {
struct Address {std::uint8_t cylinder,sector;};
struct Identity {std::uint8_t cylinder,sector,packed;};
struct Block {Identity identity;std::vector<std::uint8_t> data;std::uint8_t storedChecksum;bool checksumValid;};
RETROMEDIALAB_MEDIAENGINE_API std::uint8_t pack(std::int32_t cylinder,std::int32_t sector)noexcept;
RETROMEDIALAB_MEDIAENGINE_API Address unpack(std::uint8_t)noexcept;
RETROMEDIALAB_MEDIAENGINE_API std::optional<Identity> decodeIdentity(Flux::BitView,std::int64_t offset,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::optional<Block> decodeBlock(Flux::BitView,std::int64_t identityOffset,Identity,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API Decoding::Result decode(const Flux::Revolution&,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::vector<Decoding::Sector> decodeHardSectors(const Flux::Revolution&,bool fm,std::int32_t cylinder,std::int32_t head,std::int32_t sectorCount,std::stop_token = {});
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
 Decoding::Result decode(const Flux::Revolution& revolution)const override{return Northstar::decode(revolution);}
};
}
