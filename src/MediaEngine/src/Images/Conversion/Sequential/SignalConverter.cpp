#include "SignalConverter.h"
#include "Constants.h"
#include "../../../Formats/Tape/Common/Signal.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaEngine/DiagnosticKeys.h>
namespace RetroMediaLab::MediaEngine {
bool TapeSignalRepresentationConverter::canConvert(const MediaDocument& source,std::string_view target,RepresentationKind kind)const {
    using namespace SequentialConversionConstants;
    if(source.mediaKind()!=MediaKind::Tape||!dynamic_cast<const SequentialImage*>(&source.representation())||kind!=RepresentationKind::Sequential)return false;
    int channels{},rate{},bits{},align{};
    return ((source.formatId()==UefFormat||source.formatId()==AtomFormat)&&(target==WaveFormat||target==CswFormat||target==RawFormat))
        ||(target==UefFormat&&TapePcm::tryProfile(source.metadata(),channels,rate,bits,align));
}
SequentialMediaConversionPlan TapeSignalRepresentationConverter::convert(std::shared_ptr<const MediaDocument> source,std::string target,RepresentationKind kind,std::map<std::string,std::string> options,std::stop_token token)const {
    using namespace SequentialConversionConstants;
    checkCancellation(token);
    if(!source||!canConvert(*source,target,kind))throw MediaError(DiagnosticCode::SequentialSignalConversion);
    auto descriptor=source->mediaSource()->options();if(!options.empty())descriptor.parameters=std::move(options);
    const auto owner=std::make_shared<MediaSource>(std::move(descriptor));
    std::shared_ptr<const MediaDocument> result;ConversionMessages losses;
    if(target==UefFormat){
        const auto native=TapePcm::readChannels(*source,token);std::vector<std::int16_t> samples;
        if(native.channels.empty())throw MediaError(DiagnosticCode::TapePcmData);
        samples.reserve(native.channels.front().size());
        for(const auto value:native.channels.front()){checkCancellation(token);samples.push_back(static_cast<std::int16_t>(((value>0)-(value<0))*SignalAmplitude));}
        result=TapeSignal::toUef(owner,MediaTapeSignal(std::move(samples),native.rate),token);
        losses.push_back({std::string(DiagnosticKeys::SequentialConversionAmplitude),{},{}});
    }else{
        const auto signal=TapeSignal::fromUef(dynamic_cast<const SequentialImage&>(source->representation()),token);
        result=TapePcm::signalDocument(owner,std::move(target),signal.signal,token);
        losses.push_back({std::string(source->formatId()==UefFormat?DiagnosticKeys::SequentialConversionChunks:DiagnosticKeys::SequentialConversionAtomSignal),{},{}});
    }
    return {std::move(result),{source->diagnostics().begin(),source->diagnostics().end()},std::move(losses)};
}
}
