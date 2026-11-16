#include <RetroMediaLab/MediaFileSystems/SequentialContent.h>
namespace RetroMediaLab::MediaFileSystems {
SequentialDecodedBlock::SequentialDecodedBlock(std::int64_t position,std::vector<std::uint8_t> data,std::optional<bool> integrity,std::map<std::string,std::string> metadata)
    :position_(position),data_(std::move(data)),integrity_(integrity),metadata_(std::move(metadata)) {
    if(position_<0)throw MediaError(DiagnosticCode::SequentialBlockPosition);
}
}
