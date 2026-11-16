#include "TapeSignal.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
MediaTapeBlock::MediaTapeBlock(int position,std::vector<std::uint8_t> data,std::optional<bool> integrity,std::map<std::string,std::string> metadata)
    :position_(position),data_(std::make_shared<const std::vector<std::uint8_t>>(std::move(data))),integrity_(integrity),metadata_(std::move(metadata)){
    if(position_<0)throw MediaError(DiagnosticCode::TapePcmData);
}
MediaTapeSignal::MediaTapeSignal(std::vector<std::int16_t> samples,int rate,std::optional<std::vector<int>> positions,std::optional<std::vector<MediaTapeBlock>> blocks)
    :samples_(std::make_shared<const std::vector<std::int16_t>>(std::move(samples))),rate_(rate),positions_(std::move(positions)),blocks_(std::move(blocks)){
    if(rate_<=0||(positions_&&std::any_of(positions_->begin(),positions_->end(),[](int position){return position<0;})))throw MediaError(DiagnosticCode::TapePcmData);
}
}
