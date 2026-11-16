#include "SignalBlocks.h"
#include "SignalIoConstants.h"
#include "../Uef/Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QString>
#include <algorithm>
#include <cmath>
#include <limits>
namespace RetroMediaLab::MediaEngine::TapeSignal {
namespace {
using namespace IoConstants;
int number(std::string_view value){bool valid=false;const auto result=QString::fromUtf8(value.data(),static_cast<qsizetype>(value.size())).toInt(&valid);if(!valid)throw MediaError(DiagnosticCode::TapePcmData);return result;}
}
int carrierStart(const MediaTapeSignal& signal,int position){
    if(signal.samples().size()>static_cast<std::size_t>(std::numeric_limits<int>::max()))throw MediaError(DiagnosticCode::TapePcmData);
    const auto samples=signal.samples();position=std::clamp(position,0,static_cast<int>(samples.size()));
    int previous=position,earliest=position,edges=0;
    for(int sample=position-1;sample>0;--sample){
        if(!samples[sample])break;if((samples[sample]>0)==(samples[sample-1]>0))continue;
        const auto width=previous-sample;
        if(edges>0&&(width<signal.sampleRate()/MaximumCarrierRate||width>signal.sampleRate()/MinimumCarrierRate))break;
        earliest=previous=sample;++edges;
    }
    return edges>MinimumCarrierEdges?earliest:position;
}
MediaTapeSignal associateBlocks(const MediaTapeSignal& signal,const SequentialDecodeResult& decoded,const SequentialImage& image,const std::map<std::int64_t,int>& positions,std::stop_token token){
    checkCancellation(token);std::vector<MediaTapeBlock> blocks;std::vector<int> markers;
    for(const auto& block:decoded.blocks()){
        checkCancellation(token);const auto& metadata=block.metadata();const auto sample=metadata.find(std::string(SamplePositionKey));int position=0;
        if(sample!=metadata.end())position=number(sample->second);
        else{
            if(!block.sourcePositions().empty()){const auto found=positions.find(block.sourcePositions().front());if(found!=positions.end())position=found->second;}
            if(const auto offset=metadata.find(std::string(ByteOffsetKey));offset!=metadata.end()){
                int baud=DefaultBaud;
                if(image.segments())for(const auto& segment:*image.segments())if(std::find(block.sourcePositions().begin(),block.sourcePositions().end(),segment.fields().position)!=block.sourcePositions().end()){
                    const auto found=segment.fields().metadata.find(std::string(Uef::Constants::BaudRateMetadataKey));if(found!=segment.fields().metadata.end())baud=number(found->second);break;
                }
                const auto bytes=number(offset->second);if(bytes<0||baud<=0)throw MediaError(DiagnosticCode::TapePcmData);
                const auto adjustment=TapePcm::roundEven(double(bytes)*SerialBitsPerByte*signal.sampleRate()/baud);
                if(!std::isfinite(adjustment)||adjustment>std::numeric_limits<int>::max()-static_cast<double>(position))throw MediaError(DiagnosticCode::TapePcmData);position+=static_cast<int>(adjustment);
            }
        }
        blocks.emplace_back(position,std::vector<std::uint8_t>(block.data().begin(),block.data().end()),block.integrityValid(),metadata);
        markers.push_back(carrierStart(signal,position));
    }
    if(blocks.empty())return signal.withBlocks({},signal.blockPositions());
    std::sort(markers.begin(),markers.end());markers.erase(std::unique(markers.begin(),markers.end()),markers.end());return signal.withBlocks(std::move(blocks),std::move(markers));
}
}
