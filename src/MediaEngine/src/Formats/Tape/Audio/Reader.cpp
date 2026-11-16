#include "Reader.h"
#include "../../Common/Audio/DecodedPcm.h"
#include "../../Common/Audio/Constants.h"
#include "../../Common/Audio/Vorbis/PcmSource.h"
#include "../Common/PcmConstants.h"
#include "../../../Models/Sequential/Image.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaEngine::AudioTape {
namespace {
std::shared_ptr<const MediaDocument> document(RecognitionContext context,AudioPcm::DecodedPcm pcm,std::stop_token token){
    using namespace AudioPcm::Constants;namespace P=TapePcm::Constants;
    if(!pcm.data||pcm.channels<=0||pcm.channels>MaximumAlign/SampleBytes||pcm.sampleRate<=0)throw MediaError(DiagnosticCode::AudioPcmData);
    const auto length=pcm.data->length(),align=static_cast<std::uint64_t>(pcm.channels)*SampleBytes;
    if(!length||length%align||length>static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()))throw MediaError(DiagnosticCode::AudioPcmData);
    const auto frames=length/align;const auto ticks=double(frames)/pcm.sampleRate*TicksPerSecond;
    if(ticks>=double(std::numeric_limits<std::int64_t>::max()))throw MediaError(DiagnosticCode::AudioPcmData);
    const SequentialTime duration(static_cast<std::int64_t>(ticks));std::vector<SequentialSegment> segments;
    for(int channel=0;channel<pcm.channels;++channel){checkCancellation(token);SequentialSegmentFields fields;
        fields.position=0;fields.kind=SequentialSegmentKind::Samples;fields.length=static_cast<std::int64_t>(frames);fields.start=SequentialTime{};fields.duration=duration;fields.channelNumber=channel;fields.direction=SequentialTravelDirection::Forward;
        fields.dataRange=DataRange(0,length,DataRangeKind::Stored,pcm.data);
        fields.metadata={{std::string(ChannelKey),std::to_string(channel)},{std::string(InterleavedKey),std::string(TrueValue)},{std::string(P::AlignKey),std::to_string(align)}};segments.emplace_back(std::move(fields));
    }
    std::map<std::string,std::string> metadata{{std::string(P::ChannelsKey),std::to_string(pcm.channels)},{std::string(P::RateKey),std::to_string(pcm.sampleRate)},{std::string(P::BitsKey),std::to_string(SampleBits)},{std::string(P::AlignKey),std::to_string(align)},{std::string(ExtensionKey),context.extension()}};
    return std::make_shared<MediaDocument>(context.source(),std::string(Format),MediaKind::Tape,std::make_shared<SequentialImage>(static_cast<std::int64_t>(length),duration,std::move(segments)),std::vector<MediaFileSystems::VolumeDescriptor>{},std::vector<MediaFileSystems::MessageRecipe>{},std::move(metadata));
}
}
const ReaderCapabilities& Reader::capabilities()const{
    static const ReaderCapabilities value=[] {ReaderCapabilities result;result.formatIds.emplace_back(AudioPcm::Constants::Format);for(const auto extension:AudioPcm::Constants::Extensions)result.extensions.emplace_back(extension);result.mediaKinds={MediaKind::Tape};result.representationKinds={RepresentationKind::Sequential};return result;}();return value;
}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{checkCancellation(token);return (!context.requestedFormatId()||Reader{}.supportsFormatId(*context.requestedFormatId()))&&std::any_of(AudioPcm::Constants::Extensions.begin(),AudioPcm::Constants::Extensions.end(),[&](auto extension){return sameIdentifier(context.extension(),extension);});});}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{
    checkCancellation(token);if(!Reader{}.canReadAsync(context,token).get())throw MediaError(DiagnosticCode::AudioPcmData);
    if(sameIdentifier(context.extension(),AudioPcm::Constants::OggExtension)){auto pcm=Vorbis::openPcmSource(context.dataSource(),token);if(pcm.sampleRate>std::numeric_limits<int>::max())throw MediaError(DiagnosticCode::AudioPcmData);return document(context,{std::move(pcm.data),pcm.channels,static_cast<int>(pcm.sampleRate)},token);}
    return document(context,AudioPcm::decode(context.dataSource(),token),token);
});}
}
