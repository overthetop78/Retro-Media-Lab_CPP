#include "ProtectedImage.h"
#include "Writer.h"
#include "DiskTypeConstants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <charconv>
#include <limits>

namespace RetroMediaLab::MediaEngine::Scp {
namespace {
using namespace Constants;
using Metadata=std::map<std::string,std::string>;

std::optional<std::uint8_t> readByte(const Metadata& metadata,std::string_view key) {
    const auto found=metadata.find(std::string(key));
    if(found==metadata.end())return {};
    const auto& text=found->second;
    if(text.empty()||!std::all_of(text.begin(),text.end(),[](char c){return c>=FirstDigit&&c<=LastDigit;}))return {};
    std::uint32_t value=0;
    const auto parsed=std::from_chars(text.data(),text.data()+text.size(),value,DecimalBase);
    return parsed.ec==std::errc{}&&value<=ByteMaximum?std::optional(std::uint8_t(value)):std::nullopt;
}

std::uint8_t resolutionIndex(const Metadata& metadata,std::optional<std::int32_t> common) {
    if(const auto stored=readByte(metadata,ResolutionKey))return *stored;
    if(!common||*common%ResolutionStep)return CaptureResolution;
    const auto index=*common/ResolutionStep-1;
    return index<0||index>std::int32_t(ByteMaximum)?CaptureResolution:std::uint8_t(index);
}

std::uint32_t convertTicks(std::uint32_t ticks,std::int32_t source,std::uint32_t target) {
    const auto converted=(std::uint64_t(ticks)*std::uint32_t(source)+target/RoundingDivisor)/target;
    if(converted>MaximumInterval)throw MediaError(DiagnosticCode::ScpOverflow);
    return std::uint32_t(std::max(converted,std::uint64_t(MinimumTick)));
}
}

Image fromProtected(const Flux::ProtectedTrackImage& image,const std::map<std::string,std::string>& metadata,std::stop_token token) {
    checkCancellation(token);
    if(image.tracks().empty())throw MediaError(DiagnosticCode::ProtectedImageEmpty);
    std::size_t count=std::numeric_limits<std::size_t>::max();
    std::optional<std::int32_t> common;
    bool mixed=false;
    for(const auto& track:image.tracks()) {
        checkCancellation(token);
        count=std::min(count,track.revolutions().size());
        for(const auto& revolution:track.revolutions()) {
            const auto value=revolution.resolutionNanoseconds();
            if(!common)common=value;
            else mixed|=*common!=value;
        }
    }
    if(!count||count>Constants::ByteMaximum)throw MediaError(DiagnosticCode::ScpRevolutions,{std::to_string(count)});
    const auto resolution=resolutionIndex(metadata,mixed?std::nullopt:common);
    const auto nanoseconds=Constants::ResolutionStep*(std::uint32_t(resolution)+1);
    Image result;
    bool side0=false,side1=false;
    result.tracks.reserve(image.tracks().size());
    for(const auto& input:image.tracks()) {
        checkCancellation(token);
        if(input.head()>=std::int32_t(Constants::HeadCount))throw MediaError(DiagnosticCode::ScpHeads,{std::to_string(input.head())});
        Track track;
        track.trackNumber=trackNumber(input.cylinder(),input.head());
        track.cylinder=input.cylinder();track.head=input.head();
        side0|=input.head()==0;side1|=input.head()==1;
        track.revolutions.reserve(count);
        for(std::size_t index=0;index<count;++index) {
            const auto& source=input.revolutions()[index];
            Revolution revolution;
            revolution.flux.indexTimeTicks=convertTicks(source.flux().indexTimeTicks,source.resolutionNanoseconds(),nanoseconds);
            revolution.flux.intervals.reserve(source.flux().intervals.size());
            for(const auto interval:source.flux().intervals) {
                checkCancellation(token);
                revolution.flux.intervals.push_back(convertTicks(interval,source.resolutionNanoseconds(),nanoseconds));
            }
            if(revolution.flux.intervals.size()>Constants::MaximumInterval)throw MediaError(DiagnosticCode::ScpOverflow);
            revolution.declaredFluxCount=std::uint32_t(revolution.flux.intervals.size());
            track.revolutions.push_back(std::move(revolution));
        }
        result.tracks.push_back(std::move(track));
    }
    std::sort(result.tracks.begin(),result.tracks.end(),[](const auto& a,const auto& b){return a.trackNumber<b.trackNumber;});
    auto flags=readByte(metadata,Constants::FlagsKey).value_or(Constants::IndexAlignedFlag|Constants::ThirdPartyFlag|(image.writeProtected()?Constants::NoFlags:Constants::WritableFlag));
    flags&=Constants::RetainedFlags;
    result.header={readByte(metadata,Constants::RawVersionKey).value_or(Constants::CaptureVersion),
        readByte(metadata,Constants::DiskTypeKey).value_or(DiskTypes::Other720),std::uint8_t(count),
        result.tracks.front().trackNumber,result.tracks.back().trackNumber,flags,
        readByte(metadata,Constants::WidthKey).value_or(Constants::DefaultWidth),
        side0&&side1?Constants::BothHeads:side0?Constants::Side0:Constants::Side1,
        resolution,Constants::MissingChecksum};
    result.checksumValid=true;result.fileSize=Constants::InitialFileSize;
    return result;
}
}
