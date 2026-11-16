#include "Models.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QString>
#include <algorithm>
#include <cmath>
namespace RetroMediaLab::MediaEngine {
SequentialDecodedBlock::SequentialDecodedBlock(std::int64_t position,std::vector<std::uint8_t> data,std::vector<std::int64_t> positions,std::optional<bool> integrity,std::map<std::string,std::string> metadata)
    :position_(position),data_(std::make_shared<const std::vector<std::uint8_t>>(std::move(data))),positions_(std::move(positions)),integrity_(integrity),metadata_(std::move(metadata)) {
    if(position_<0||std::any_of(positions_.begin(),positions_.end(),[](auto value){return value<0;}))throw MediaError(DiagnosticCode::SequentialDecodeModel);
}
SequentialDecodedBlock SequentialDecodedBlock::withSourcePositions(std::vector<std::int64_t> positions)const{if(std::any_of(positions.begin(),positions.end(),[](auto value){return value<0;}))throw MediaError(DiagnosticCode::SequentialDecodeModel);auto result=*this;result.positions_=std::move(positions);return result;}
SequentialDecodeResult::SequentialDecodeResult(std::string id,double confidence,std::vector<SequentialDecodedBlock> blocks,std::vector<SequentialSegment> undecoded,std::vector<std::string> diagnostics,std::vector<MediaFileSystems::MessageRecipe> messages)
    :decoderId_(std::move(id)),confidence_(confidence),blocks_(std::move(blocks)),undecoded_(std::move(undecoded)),diagnostics_(std::move(diagnostics)),messages_(std::move(messages)) {
    if(QString::fromStdString(decoderId_).trimmed().isEmpty()||!std::isfinite(confidence_)||confidence_<0||confidence_>1)throw MediaError(DiagnosticCode::SequentialDecodeModel);
}
}
