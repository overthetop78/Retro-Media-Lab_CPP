#include "Adapter.h"
#include "Constants.h"
#include "../../Images/WriterAdapters.h"
#include "Apridisk.h"
#include "../../Images/DocumentFactory.h"
#include "../../Primitives/Identifiers.h"
#include "../../Models/Sectors/Representation.h"
#include "../../Models/Sectors/SectorImages.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Apridisk {
const ReaderCapabilities& Reader::capabilities()const{static const ReaderCapabilities result{{std::string(Constants::FormatId)},{std::string(Constants::Extension)},{{Constants::Signature.begin(),Constants::Signature.end()}},{},{MediaKind::Floppy},{RepresentationKind::Sectors}};return result;}
bool Reader::supportsFormatId(std::string_view id)const{return Primitives::ordinalIdentifierEqualsIgnoreCase(id,Constants::FormatId);}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{const auto requested=context.requestedFormatId();if(requested&&!Reader{}.supportsFormatId(*requested))return false;if(!Primitives::ordinalIdentifierEqualsIgnoreCase(context.extension(),Constants::Extension)||context.length()<Constants::HeaderSize)return false;const auto header=context.readHeaderAsync(std::int32_t(Constants::Signature.size()),token).get();return std::equal(header.begin(),header.end(),Constants::Signature.begin(),Constants::Signature.end());});}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{const auto bytes=context.readBytesAsync(token).get();return floppySectorDocument(context.source(),read(bytes,token));});}
std::shared_ptr<const ImageWriter> writerAdapter(){
    return sectorWriterAdapter(std::string(Constants::WriterId),{std::string(Constants::FormatId)},{std::string(Constants::Extension)},
        [](const auto& image,const auto&,auto,auto){return canWrite(image);},
        [](const auto&,const MediaDocument& document,const std::filesystem::path& output,auto,std::stop_token token){
            const auto& representation=static_cast<const SectorRepresentation&>(document.representation());
            const auto image=relabelSectorImage(representation.imageOwner(),std::string(Constants::FormatId));
            write(*image,output,token);return std::vector<std::filesystem::path>{output};
        });
}
}
