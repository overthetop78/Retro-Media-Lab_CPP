#include "Reader.h"
#include "Constants.h"
#include "../../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <charconv>
#include <array>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::WavTape {
namespace {
using namespace Constants;
struct Profile {std::uint16_t channels=0,bits=0,align=0;std::uint32_t rate=0;};
bool number(const TapePcm::Parameters& values,std::string_view key,std::uint32_t& value) {
    const auto found=values.find(std::string(key));if(found==values.end()||found->second.empty())return false;
    const auto& text=found->second;const auto parsed=std::from_chars(text.data(),text.data()+text.size(),value);return parsed.ec==std::errc{}&&parsed.ptr==text.data()+text.size();
}
bool select(const MediaDocument& document,Profile& profile,std::vector<DataRange>& ranges) {
    using namespace TapePcm::Constants;std::uint32_t channels=0,rate=0,bits=0,align=0;const auto& values=document.metadata();
    if(!number(values,ChannelsKey,channels)||!number(values,RateKey,rate)||!number(values,BitsKey,bits)||!number(values,AlignKey,align)||!channels||channels>MaximumChannels||!rate||rate>Constants::MaximumRate||std::find(SupportedBits.begin(),SupportedBits.end(),bits)==SupportedBits.end()||align!=channels*(bits/ByteBits))return false;
    const auto* image=dynamic_cast<const SequentialImage*>(&document.representation());if(!image||!image->segments())return false;
    for(const auto& segment:*image->segments()) {
        const auto& value=segment.fields();if(value.kind!=SequentialSegmentKind::Samples||!value.dataRange||!value.dataRange->source())return false;
        if(!value.channelNumber||*value.channelNumber==0){if(value.dataRange->kind()!=DataRangeKind::Stored||value.dataRange->length()%align)return false;ranges.push_back(*value.dataRange);}
    }
    profile={std::uint16_t(channels),std::uint16_t(bits),std::uint16_t(align),rate};return true;
}
}
const WriterCapabilities& Writer::capabilities()const {
    static const WriterCapabilities value{std::string(WriterId),{std::string(FormatId)},{std::string(Extension)},{RepresentationKind::Sequential},false};return value;
}
bool Writer::canWrite(const MediaDocument& document,std::string_view format,std::string_view extension)const {
    Profile profile;std::vector<DataRange> ranges;return sameIdentifier(format,FormatId)&&sameIdentifier(extension,Extension)&&document.mediaKind()==MediaKind::Tape&&select(document,profile,ranges);
}
std::future<std::vector<std::filesystem::path>> Writer::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path path,std::string format,std::stop_token token)const {
    return std::async(std::launch::deferred,[document=std::move(document),path=std::move(path),format=std::move(format),token] {
        checkCancellation(token);if(!document)throw MediaError(DiagnosticCode::WaveTapeWrite);
        const auto extension=path.extension().u8string();Profile profile;std::vector<DataRange> ranges;
        if(!Writer{}.canWrite(*document,format,std::string(extension.begin(),extension.end()))||!select(*document,profile,ranges))throw MediaError(DiagnosticCode::WaveTapeWrite);
        std::uint64_t length=0;for(const auto& range:ranges){if(std::uint64_t(range.length())>MaximumRiffLength-length)throw MediaError(DiagnosticCode::WaveTapeWrite);length+=range.length();}
        const auto padding=length&1u;const auto riffLength=length+padding+OutputHeaderSize-ChunkHeaderSize;
        if(riffLength>MaximumRiffLength)throw MediaError(DiagnosticCode::WaveTapeWrite);
        writeAtomicImage(path,[&](const auto& output,auto stop) {
            std::array<std::uint8_t,OutputHeaderSize> header{};
            const auto identifier=[&](int offset,std::string_view id){std::copy(id.begin(),id.end(),header.begin()+offset);};
            identifier(0,Riff);qToLittleEndian(std::uint32_t(riffLength),header.data()+LengthOffset);identifier(WaveOffset,Wave);identifier(OutputFormatOffset,Format);qToLittleEndian(std::uint32_t(FormatSize),header.data()+OutputFormatLengthOffset);qToLittleEndian(PcmTag,header.data()+OutputTagOffset);qToLittleEndian(profile.channels,header.data()+OutputChannelsOffset);qToLittleEndian(profile.rate,header.data()+OutputRateOffset);qToLittleEndian(std::uint32_t(profile.rate*profile.align),header.data()+OutputByteRateOffset);qToLittleEndian(profile.align,header.data()+OutputAlignOffset);qToLittleEndian(profile.bits,header.data()+OutputBitsOffset);identifier(OutputDataOffset,Data);qToLittleEndian(std::uint32_t(length),header.data()+OutputLengthOffset);output(header);
            std::array<std::uint8_t,BufferSize> buffer{};
            for(const auto& range:ranges)for(std::int64_t position=0;position<range.length();) {
                checkCancellation(stop);const auto count=std::size_t(std::min<std::int64_t>(buffer.size(),range.length()-position));auto bytes=std::span(buffer).first(count);range.source()->readExactly(range.sourceOffset()+position,bytes,stop);output(bytes);position+=count;
            }
            if(padding){const std::array<std::uint8_t,1> zero{};output(zero);}
        },token);
        return std::vector<std::filesystem::path>{path};
    });
}
}
