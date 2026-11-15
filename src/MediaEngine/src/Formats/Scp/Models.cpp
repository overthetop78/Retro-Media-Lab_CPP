#include "Scp.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Scp {
std::string Header::versionText() const {
    return std::to_string(version>>Constants::VersionShift)+std::string(Constants::VersionSeparator)+std::to_string(version&Constants::VersionMask);
}
double Revolution::durationMilliseconds(std::int32_t resolution) const {
    if(resolution<=0)throw MediaError(DiagnosticCode::ScpResolution,{std::to_string(resolution)});
    return double(flux.indexTimeTicks)*resolution/Constants::NanosecondsPerMillisecond;
}
double Revolution::rpm(std::int32_t resolution) const {return Constants::MillisecondsPerMinute/durationMilliseconds(resolution);}
std::uint32_t updateChecksum(std::uint32_t checksum,MediaFileSystems::Bytes data) noexcept {
    for(const auto byte:data)checksum+=byte;
    return checksum;
}
bool isChecksumValid(std::uint32_t declared,std::uint8_t flags,std::uint32_t computed) noexcept {
    return (!declared&&(flags&Constants::WritableFlag))||declared==computed;
}
std::pair<std::int32_t,std::int32_t> trackAddress(std::int32_t number) {
    if(number<0||number>=Constants::TrackSlots)throw MediaError(DiagnosticCode::ScpTrackAddress,{std::to_string(number)});
    return {number/Constants::HeadCount,number%Constants::HeadCount};
}
}
