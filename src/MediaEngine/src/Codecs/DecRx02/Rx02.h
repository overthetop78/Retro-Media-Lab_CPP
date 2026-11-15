#pragma once
#include "Constants.h"
#include "../../Models/Flux/Flux.h"
#include "../../Encoding/Track.h"
#include "../../Decoding/Decoder.h"
#include "../FmAddressMarks.h"
#include <span>
namespace RetroMediaLab::MediaEngine::DecRx02 {
enum class DataEncoding {Fm,M2Fm};
struct SectorFormat {DataEncoding encoding;std::uint8_t sizeCode;};
struct DataMark {std::uint8_t mark;FmAddressMarks::Pattern pattern;DataEncoding encoding;bool deleted;std::int32_t sectorSize;std::uint8_t sizeCode;};
RETROMEDIALAB_MEDIAENGINE_API std::span<const DataMark> dataMarks()noexcept;
RETROMEDIALAB_MEDIAENGINE_API SectorFormat encodingForSize(std::int32_t);
RETROMEDIALAB_MEDIAENGINE_API const DataMark& dataMarkFor(DataEncoding,bool deleted)noexcept;
RETROMEDIALAB_MEDIAENGINE_API void encodeM2Fm(Flux::Bits&,std::stop_token = {});
RETROMEDIALAB_MEDIAENGINE_API std::optional<std::vector<std::uint8_t>> decodeM2Fm(Flux::BitView,std::int64_t start,std::int64_t count,std::stop_token = {});
struct Header {std::uint8_t cylinder,head,sector,sizeCode;bool crcValid;};
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
 Decoding::Result decode(const Flux::Revolution& revolution)const override{return DecRx02::decode(revolution);}
};

}
