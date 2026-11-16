#include "Profile.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <charconv>
namespace RetroMediaLab::MediaEngine::CommodoreTap {
using namespace Constants;
std::optional<int> clockRate(std::uint8_t platform,std::uint8_t video){for(const auto& value:TimerClockRates)if(value.platform==platform&&value.video==video)return value.rate;return {};}
bool integer(const std::map<std::string,std::string>& metadata,std::string_view key,int& value){
    const auto found=metadata.find(std::string(key));if(found==metadata.end())return false;
    const auto& text=found->second;unsigned number{};const auto result=std::from_chars(text.data(),text.data()+text.size(),number,DecimalRadix);
    if(result.ec!=std::errc{}||result.ptr!=text.data()+text.size()||number>static_cast<unsigned>(std::numeric_limits<int>::max()))return false;value=static_cast<int>(number);return true;
}
bool profile(const std::map<std::string,std::string>& metadata,Profile& output){
    const auto signature=metadata.find(std::string(SignatureMetadataKey));if(signature==metadata.end()||(signature->second!=C64Signature&&signature->second!=C16Signature))return false;
    int version{},platform{},video{};
    if(!integer(metadata,VersionMetadataKey,version)||version>HalfWaveVersion||!integer(metadata,PlatformMetadataKey,platform)||platform>MaximumByte||!integer(metadata,VideoStandardMetadataKey,video)||video>MaximumByte)return false;
    const auto clock=clockRate(static_cast<std::uint8_t>(platform),static_cast<std::uint8_t>(video));if(!clock)return false;
    output={signature->second,static_cast<std::uint8_t>(version),static_cast<std::uint8_t>(platform),static_cast<std::uint8_t>(video),*clock};return true;
}
Profile readHeader(const RecognitionContext& context,std::stop_token token){
    checkCancellation(token);if(context.length()<HeaderSize)throw MediaError(DiagnosticCode::CommodoreTapHeader);
    std::array<std::uint8_t,HeaderSize> bytes{};context.dataSource()->readExactly(0,bytes,token);const std::string signature(bytes.begin()+SignatureOffset,bytes.begin()+SignatureLength);
    if(signature!=C64Signature&&signature!=C16Signature)throw MediaError(DiagnosticCode::CommodoreTapHeader);
    const auto version=bytes[VersionOffset];if(version>HalfWaveVersion)throw MediaError(DiagnosticCode::CommodoreTapVersion,{std::to_string(version)});
    const auto platform=bytes[PlatformOffset],video=bytes[VideoStandardOffset];const auto clock=clockRate(platform,video);if(!clock)throw MediaError(DiagnosticCode::CommodoreTapClock);
    if(qFromLittleEndian<std::uint32_t>(bytes.data()+DataLengthOffset)!=context.length()-HeaderSize)throw MediaError(DiagnosticCode::CommodoreTapHeader);
    return {signature,version,platform,video,*clock};
}
}
