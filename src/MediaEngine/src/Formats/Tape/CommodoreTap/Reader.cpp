#include "Reader.h"
#include "Profile.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::CommodoreTap {
namespace {
using namespace Constants;
class Cursor {
public:
    explicit Cursor(const RecognitionContext& context,std::stop_token token):context_(context),token_(token){}
    std::int64_t position=HeaderSize;
    std::uint8_t next(){
        checkCancellation(token_);if(position>=context_.length())throw MediaError(DiagnosticCode::CommodoreTapData);
        if(index_==bytes_.size()){
            bytes_.resize(static_cast<std::size_t>(std::min<std::int64_t>(BufferSize,context_.length()-position)));context_.dataSource()->readExactly(position,bytes_,token_);index_=0;
        }
        ++position;return bytes_[index_++];
    }
private:const RecognitionContext& context_;std::stop_token token_;std::vector<std::uint8_t> bytes_;std::size_t index_{};
};
}
const ReaderCapabilities& Reader::capabilities()const{static const ReaderCapabilities caps{{std::string(FormatId)},{std::string(Extension)},{{C64Signature.begin(),C64Signature.end()},{C16Signature.begin(),C16Signature.end()}},{},{MediaKind::Tape},{RepresentationKind::Sequential}};return caps;}
bool Reader::supportsFormatId(std::string_view id)const{return sameIdentifier(id,FormatId);}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{
    if(context.requestedFormatId()&&!Reader{}.supportsFormatId(*context.requestedFormatId()))return false;
    try{readHeader(context,token);return true;}catch(const MediaError& error){switch(error.diagnostic().code){case DiagnosticCode::CommodoreTapHeader:case DiagnosticCode::CommodoreTapVersion:case DiagnosticCode::CommodoreTapClock:return false;default:throw;}}
});}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{
    const auto header=readHeader(context,token);Cursor input(context,token);std::vector<SequentialSegment> output;SequentialTime elapsed{};
    while(input.position<context.length()){
        checkCancellation(token);const auto offset=input.position;const auto first=input.next();int cycles{};
        if(first)cycles=first*ShortPulseCycleMultiplier;else if(header.version==OriginalVersion)cycles=Version0OverflowCycles;
        else{const auto low=input.next(),middle=input.next(),high=input.next();cycles=low|(int(middle)<<ByteBits)|(int(high)<<WordShift);if(!cycles)throw MediaError(DiagnosticCode::CommodoreTapData);}
        const auto duration=SequentialTime(static_cast<std::int64_t>((cycles/double(header.clock))*SequentialTicksPerSecond));
        if(duration.count()>std::numeric_limits<std::int64_t>::max()-elapsed.count())throw MediaError(DiagnosticCode::CommodoreTapeLimit);
        SequentialSegmentFields value;value.position=output.size();value.kind=SequentialSegmentKind::Pulse;value.length=cycles;value.start=elapsed;value.duration=duration;value.direction=SequentialTravelDirection::Forward;value.dataRange=DataRange(offset,input.position-offset,DataRangeKind::Stored,context.dataSource(),offset);
        value.metadata={{std::string(PulseMetadataKey),std::string(TrueValue)},{std::string(CycleCountMetadataKey),std::to_string(cycles)},{std::string(EncodedLengthMetadataKey),std::to_string(input.position-offset)},{std::string(HalfWaveMetadataKey),std::string(header.version==HalfWaveVersion?TrueValue:FalseValue)}};
        output.emplace_back(std::move(value));elapsed+=duration;
    }
    return std::make_shared<const MediaDocument>(context.source(),std::string(FormatId),MediaKind::Tape,std::make_shared<SequentialImage>(context.length(),elapsed,std::move(output)),std::vector<MediaFileSystems::VolumeDescriptor>{},std::vector<MediaFileSystems::MessageRecipe>{},
        std::map<std::string,std::string>{{std::string(SignatureMetadataKey),header.signature},{std::string(VersionMetadataKey),std::to_string(header.version)},{std::string(PlatformMetadataKey),std::to_string(header.platform)},{std::string(VideoStandardMetadataKey),std::to_string(header.video)},{std::string(ClockRateMetadataKey),std::to_string(header.clock)}});
});}
}
