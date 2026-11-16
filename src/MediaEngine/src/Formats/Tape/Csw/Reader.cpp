#include "Reader.h"
#include "Constants.h"
#include "Compression.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaEngine::Csw {
namespace {
using namespace Constants;
bool header(std::span<const std::uint8_t> bytes){return bytes.size()>=HeaderV1&&std::equal(Signature.begin(),Signature.end(),bytes.begin())&&bytes[TerminatorOffset]==Terminator&&(bytes[MajorOffset]==Version1||bytes[MajorOffset]==Version2);}
class PulseDecoder {
public:
    explicit PulseDecoder(bool high):high_(high){}
    void append(std::span<const std::uint8_t> bytes,std::stop_token token){for(const auto value:bytes){checkCancellation(token);if(remaining_){length_|=std::uint32_t(value)<<(ByteBits*(ExtendedBytes-remaining_));if(--remaining_==0)emitPulse();}else if(value){length_=value;emitPulse();}else{length_=0;remaining_=ExtendedBytes;}}}
    void finish()const{if(remaining_)throw MediaError(DiagnosticCode::CswPulse);}
    std::vector<std::int16_t> samples;std::uint32_t count{};
private:
    void emitPulse(){if(!length_||length_>MaximumSamples-samples.size())throw MediaError(DiagnosticCode::CswPulse);samples.insert(samples.end(),length_,high_?Amplitude:std::int16_t(-Amplitude));high_=!high_;++count;}
    bool high_;std::uint32_t length_{};std::size_t remaining_{};
};
}
const ReaderCapabilities& Reader::capabilities()const{static const ReaderCapabilities caps{{std::string(FormatId)},{std::string(Extension)},{{Signature.begin(),Signature.end()}},{},{MediaKind::Tape},{RepresentationKind::Sequential}};return caps;}
bool Reader::supportsFormatId(std::string_view id)const{return id==FormatId;}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{if(context.requestedFormatId()&&!Reader{}.supportsFormatId(*context.requestedFormatId()))return false;return header(context.readHeaderAsync(HeaderV1,token).get());});}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{
    const auto bytes=context.readBytesAsync(token).get();if(!header(bytes))throw MediaError(DiagnosticCode::CswHeader);const bool version2=bytes[MajorOffset]==Version2;if(version2&&bytes.size()<HeaderV2)throw MediaError(DiagnosticCode::CswHeader);
    const auto rate=version2?qFromLittleEndian<std::uint32_t>(bytes.data()+RateOffset):qFromLittleEndian<std::uint16_t>(bytes.data()+RateOffset);if(!rate||rate>static_cast<std::uint32_t>(std::numeric_limits<int>::max()))throw MediaError(DiagnosticCode::CswHeader);
    const auto compression=bytes[version2?CompressionV2:CompressionV1];const auto offset=version2?HeaderV2+bytes[ExtensionOffset]:HeaderV1;if(offset>bytes.size())throw MediaError(DiagnosticCode::CswHeader);
    PulseDecoder decoder((bytes[version2?FlagsV2:FlagsV1]&HighFlag)!=0);const auto payload=std::span<const std::uint8_t>(bytes).subspan(offset);
    if(compression==Rle)decoder.append(payload,token);else if(version2&&compression==Zrle)inflatePulses(payload,[&](auto chunk){decoder.append(chunk,token);},token);else throw MediaError(DiagnosticCode::CswCompression);
    decoder.finish();if(version2&&decoder.count!=qFromLittleEndian<std::uint32_t>(bytes.data()+CountOffset))throw MediaError(DiagnosticCode::CswCount);
    return TapePcm::signalDocument(context.source(),std::string(FormatId),decoder.samples,static_cast<int>(rate),token);
});}
}
