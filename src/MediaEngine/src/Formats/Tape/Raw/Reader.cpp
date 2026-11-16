#include "Reader.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::RawTape {
const ReaderCapabilities& Reader::capabilities()const{using namespace TapePcm::Constants;static const ReaderCapabilities caps{{std::string(RawFormat)},{std::string(RawExtension)},{},{},{MediaKind::Tape},{RepresentationKind::Sequential}};return caps;}
bool Reader::supportsFormatId(std::string_view id)const{return id==TapePcm::Constants::RawFormat;}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{checkCancellation(token);return context.requestedFormatId()==TapePcm::Constants::RawFormat;});}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{const auto& parameters=context.source()->options().parameters;const auto profile=TapePcm::readProfile(parameters?&*parameters:nullptr);const auto bytes=context.readBytesAsync(token).get();return TapePcm::rawDocument(context.source(),bytes,profile,token);});}
}
