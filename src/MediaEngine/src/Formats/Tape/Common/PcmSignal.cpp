#include "Signal.h"
#include "SignalConstants.h"
#include "../../../Models/Blocks/Internal.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <bit>
namespace RetroMediaLab::MediaEngine::TapeSignal {
MediaTapeSignal pcm(const MediaDocument& document,std::stop_token token){
    checkCancellation(token);
    using namespace TapeSignalConstants;
    int channels{},rate{},bits{},align{};
    const auto* image=dynamic_cast<const SequentialImage*>(&document.representation());
    if(!image||!TapePcm::tryProfile(document.metadata(),channels,rate,bits,align))throw MediaError(DiagnosticCode::TapePcmData);
    std::vector<std::int16_t> samples;
    if(image->segments())for(const auto& segment:*image->segments()){
        checkCancellation(token);
        const auto& value=segment.fields();
        if(value.kind!=SequentialSegmentKind::Samples||value.channelNumber!=0)continue;
        const auto bytes=readStoredRange(value.dataRange?&*value.dataRange:nullptr,token).value_or(std::vector<std::uint8_t>{});
        const auto values=TapePcm::extractChannel(bytes,channels,bits,align,0);
        if(values.size()>std::size_t(TapePcm::Constants::MaximumBuffer)-samples.size())throw MediaError(DiagnosticCode::TapePcmData);
        for(const auto sample:values){
            const auto reduced=bits<SignalBits?std::bit_cast<std::int32_t>(std::uint32_t(sample)<<(SignalBits-bits)):sample>>(bits-SignalBits);
            samples.push_back(static_cast<std::int16_t>(reduced));
        }
    }
    return MediaTapeSignal(std::move(samples),rate);
}
}
