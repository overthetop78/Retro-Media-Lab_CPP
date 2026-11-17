#include <RetroMediaLab/MediaEngine/MediaReading.h>
#include "../Composition/Runtime.h"
#include "../Formats/Tape/Common/Pcm.h"
namespace RetroMediaLab::MediaEngine {
TapePcmProfile readTapePcmProfile(const std::map<std::string,std::string>& parameters){const auto value=TapePcm::readProfile(&parameters);return {value.sampleRate,value.channels,value.bits,value.signedSamples,value.littleEndian};}
std::map<std::string,std::string> tapePcmParameters(const TapePcmProfile& value){
    namespace C=TapePcm::Constants;
    TapePcm::validate({value.sampleRate,value.channels,value.bitsPerSample,value.signedSamples,value.littleEndian});
    return {{std::string(C::RateKey),std::to_string(value.sampleRate)},{std::string(C::ChannelsKey),std::to_string(value.channels)},{std::string(C::BitsKey),std::to_string(value.bitsPerSample)},{std::string(C::SignedKey),std::string(value.signedSamples?C::TrueValue:C::FalseValue)},{std::string(C::EndianKey),std::string(value.littleEndian?C::TrueValue:C::FalseValue)}};
}
std::future<std::shared_ptr<const MediaDocument>> readMediaDocumentAsync(std::filesystem::path path,std::optional<std::string> format,std::map<std::string,std::string> parameters,std::stop_token token){
    SourceOptions options;const auto utf8=path.u8string();options.primaryPath.assign(utf8.begin(),utf8.end());options.requestedFormatId=std::move(format);options.parameters=std::move(parameters);
    return RuntimeDetails::defaultServices()->reading.readingService().readAsync(std::make_shared<MediaSource>(std::move(options)),token);
}
}
