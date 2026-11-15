#include "ImgReader.h"
#include "RawImages.h"
#include "Constants.h"
#include "../../Images/DocumentFactory.h"
#include "../../Models/Sectors/SectorImages.h"
#include "../../Sources/RandomAccessSources.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaFileSystems/Fat.h>
#include <RetroMediaLab/MediaFileSystems/Cpm.h>
namespace RetroMediaLab::MediaEngine::Raw {
namespace {
std::shared_ptr<const MediaFileSystems::SectorImage> readImg(MediaFileSystems::Bytes bytes,std::stop_token token){const auto bpb=MediaFileSystems::Fat::detectBpbGeometry(bytes,bytes.size());auto image=readIbm(memorySource(bytes),token);if(!bpb&&MediaFileSystems::Cpm::looksLikeCpcSystemDirectory(*image,token))return relabelSectorImage(image,std::string(MediaFileSystems::Formats::AmstradCpc));if(!bpb&&MediaFileSystems::Cpm::parseAmstradSpecification(bytes))return relabelSectorImage(image,std::string(MediaFileSystems::Formats::AmstradPcw));return image;}
}
const ReaderCapabilities& ImgReader::capabilities()const{static const auto result=[]{ReaderCapabilities c;for(const auto& g:ibmGeometries())c.formatIds.push_back(g.formatId);c.formatIds.emplace_back(MediaFileSystems::Formats::AmstradCpc);c.formatIds.emplace_back(MediaFileSystems::Formats::AmstradPcw);c.extensions={std::string(Constants::Img)};c.mediaKinds={MediaKind::Floppy};c.representationKinds={RepresentationKind::Sectors};return c;}();return result;}
bool ImgReader::supportsFormatId(std::string_view id)const{return ImageReader::supportsFormatId(id)||Primitives::ordinalIdentifierStartsWithIgnoreCase(id,Constants::IbmPrefix);}
std::future<bool> ImgReader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{const auto requested=context.requestedFormatId();if(requested&&!ImgReader{}.supportsFormatId(*requested))return false;const auto bytes=context.readBytesAsync(token).get();try{readImg(bytes,token);return true;}catch(const MediaError& e){if(e.diagnostic().code==DiagnosticCode::RawInvalidLength||e.diagnostic().code==DiagnosticCode::RawUnknownGeometry)return false;throw;}catch(const MediaFileSystems::MediaError& e){if(e.kind()==MediaFileSystems::ErrorKind::InvalidData)return false;throw;}});}
std::future<std::shared_ptr<const MediaDocument>> ImgReader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{const auto bytes=context.readBytesAsync(token).get();return floppySectorDocument(context.source(),readImg(bytes,token));});}
}
