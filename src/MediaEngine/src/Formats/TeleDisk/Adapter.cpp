#include "Reader.h"
#include "Constants.h"
#include "../Raw/RawImages.h"
#include "../Raw/Constants.h"
#include "../../Images/DocumentFactory.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::TeleDisk {
const ReaderCapabilities& Reader::capabilities()const{static const auto result=[]{ReaderCapabilities caps;for(const auto& geometry:Raw::ibmGeometries())caps.formatIds.push_back(geometry.formatId);caps.formatIds.emplace_back(MediaFileSystems::Formats::IbmScan);caps.formatIds.emplace_back(MediaFileSystems::Formats::UcsdIbmMfm);caps.extensions={std::string(Constants::Extension)};caps.signatures={{Constants::Signature.begin(),Constants::Signature.end()},{Constants::AdvancedSignature.begin(),Constants::AdvancedSignature.end()}};caps.mediaKinds={MediaKind::Floppy};caps.representationKinds={RepresentationKind::Sectors};return caps;}();return result;}
bool Reader::supportsFormatId(std::string_view id)const{return ImageReader::supportsFormatId(id)||Primitives::ordinalIdentifierStartsWithIgnoreCase(id,Raw::Constants::IbmPrefix);}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{const auto requested=context.requestedFormatId();if(requested&&!Reader{}.supportsFormatId(*requested))return false;if(context.length()<Constants::HeaderSize)return false;const auto bytes=context.readHeaderAsync(std::int32_t(Constants::Signature.size()),token).get();return std::equal(bytes.begin(),bytes.end(),Constants::Signature.begin(),Constants::Signature.end())||std::equal(bytes.begin(),bytes.end(),Constants::AdvancedSignature.begin(),Constants::AdvancedSignature.end());});}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{const auto bytes=context.readBytesAsync(token).get();return floppySectorDocument(context.source(),readDetailed(bytes,token).sectorImage);});}
}
