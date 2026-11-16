#include "Adapter.h"
#include "Representation.h"
#include "Constants.h"
#include "../../../Sources/RandomAccessSources.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::MfmEmu {
const ReaderCapabilities& Reader::capabilities()const{using namespace Constants;static const ReaderCapabilities caps{{std::string(FormatId)},{std::string(Extension)},{std::vector<std::uint8_t>(Signature.begin(),Signature.end())},{},{MediaKind::HardDisk},{RepresentationKind::Flux}};return caps;}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{checkCancellation(token);if((context.requestedFormatId()&&!Reader{}.supportsFormatId(*context.requestedFormatId()))||context.length()<Constants::FixedHeaderLength)return false;const auto bytes=context.readHeaderAsync(std::int32_t(Constants::Signature.size()),token).get();return std::equal(bytes.begin(),bytes.end(),Constants::Signature.begin(),Constants::Signature.end());});}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{if(!Reader{}.canReadAsync(context,token).get())throw MediaError(DiagnosticCode::MfmHeader);const auto& path=context.source()->primaryPath();auto container=Container::open(fileSource(std::filesystem::path(std::u8string(path.begin(),path.end()))),token);return std::make_shared<const MediaDocument>(context.source(),std::string(Constants::FormatId),MediaKind::HardDisk,std::make_shared<const Representation>(std::move(container)));});}
}
