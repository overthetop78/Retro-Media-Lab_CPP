#include "I86f.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <bit>
namespace RetroMediaLab::MediaEngine::I86f {
namespace {
using namespace Constants;
std::uint16_t u16(std::span<const std::uint8_t> data,std::size_t offset){return std::uint16_t(data[offset]|std::uint32_t(data[offset+1])<<BitsPerByte);}
std::uint32_t u32(std::span<const std::uint8_t> data,std::size_t offset){std::uint32_t value{};for(std::size_t i=0;i<TableEntryBytes;++i)value|=std::uint32_t(data[offset+i])<<(i*BitsPerByte);return value;}
std::size_t trackOffset(std::span<const std::uint8_t> bytes,std::size_t logical) {
    const auto value=u32(bytes,TableOffset+logical*TableEntryBytes);
    if(value>MaximumOffset)throw MediaError(DiagnosticCode::I86fOffset,{std::to_string(logical),std::to_string(value),std::to_string(bytes.size())});return value;
}
std::optional<Track> readTrack(std::span<const std::uint8_t> bytes,std::size_t logical,std::size_t offset,std::size_t next,std::uint16_t flags,std::stop_token token) {
    const bool extra=(flags&ExtraBitCellCount)!=0;const auto header=extra?ExtendedHeaderBytes:StandardHeaderBytes;
    if(offset>bytes.size()||header>bytes.size()-offset||next<offset+header)throw MediaError(DiagnosticCode::I86fRange,{std::to_string(logical),std::to_string(offset),std::to_string(next),std::to_string(header),std::to_string(bytes.size())});
    const auto trackFlags=u16(bytes,offset+TrackFlagsOffset);
    const bool explicitCount=extra&&(flags&SpeedShiftMask)==NoFlags&&(flags&SpeedupOrExplicitBitCount)!=0;
    const auto count=explicitCount?std::int64_t(std::bit_cast<std::int32_t>(u32(bytes,offset+ExplicitBitCountOffset))):std::int64_t(next-offset-header)*std::int64_t(BitsPerByte);
    if(count<=0||count>MaximumOffset)throw MediaError(DiagnosticCode::I86fBitCount,{std::to_string(logical),std::to_string(count)});
    const auto byteCount=(std::uint64_t(count)+WordBits-1)/WordBits*BytesPerWord;const auto available=bytes.size()-offset-header;
    if(byteCount>available)throw MediaError(DiagnosticCode::I86fTruncated,{std::to_string(logical),std::to_string(offset),std::to_string(next),std::to_string(byteCount),std::to_string(available)});
    const auto source=bytes.subspan(offset+header,std::size_t(byteCount));const bool reverse=(flags&ReverseByteOrder)!=0;
    Flux::Bits bits(std::size_t(count),std::uint8_t{});bool any=false;
    for(std::size_t bit=0;bit<bits.size();++bit){checkCancellation(token);const auto wordByte=bit/WordBits*BytesPerWord;auto byteInWord=bit/BitsPerByte%BytesPerWord;
        if(reverse)byteInWord^=BytesPerWord-1;
        const auto value=(source[wordByte+byteInWord]&(MostSignificantBit>>(bit%BitsPerByte)))!=0;bits[bit]=std::uint8_t(value);any|=value;
    }
    if(!any)return {};return Track{std::int32_t(logical),trackFlags,std::int32_t(count),std::move(bits)};
}
}
Image read(std::span<const std::uint8_t> bytes,std::stop_token token) {
    using namespace Constants;checkCancellation(token);
    if(bytes.size()<MinimumFileLength||!std::equal(Signature.begin(),Signature.end(),bytes.begin()))throw MediaError(DiagnosticCode::I86fSignature,{std::to_string(bytes.size())});
    const auto flags=u16(bytes,FileFlagsOffset);const auto count=(flags&TwoSided)!=0?TwoSideEntries:EntriesPerSide;const auto tableEnd=TableOffset+count*TableEntryBytes;
    if(bytes.size()<tableEnd)throw MediaError(DiagnosticCode::I86fTable,{std::to_string(tableEnd),std::to_string(bytes.size())});
    Image image{flags,{}};
    for(std::size_t logical=0;logical<count;++logical){checkCancellation(token);const auto offset=trackOffset(bytes,logical);if(offset==0)continue;
        auto next=bytes.size();for(auto index=logical+1;index<count;++index){const auto value=trackOffset(bytes,index);if(value!=0){next=value;break;}}
        if(auto track=readTrack(bytes,logical,offset,next,flags,token))image.tracks.push_back(std::move(*track));
    }return image;
}
}
