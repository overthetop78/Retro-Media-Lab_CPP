#include "Adapter.h"
#include "Constants.h"
#include "../../Images/WriterAdapters.h"
#include "Rx02.h"
#include "../../Images/DocumentFactory.h"
#include "../../Primitives/Identifiers.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Rx02 {
const ReaderCapabilities& Reader::capabilities()const{static const ReaderCapabilities result{{std::string(Constants::FormatId)},{std::string(Constants::Extension)},{},{},{MediaKind::Floppy},{RepresentationKind::Sectors}};return result;}
bool Reader::supportsFormatId(std::string_view id)const{return Primitives::ordinalIdentifierEqualsIgnoreCase(id,Constants::FormatId);}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{MediaFileSystems::checkCancellation(token);const auto requested=context.requestedFormatId();if(requested&&!Reader{}.supportsFormatId(*requested))return false;return context.length()==Constants::Capacity;});}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{const auto bytes=context.readBytesAsync(token).get();return floppySectorDocument(context.source(),read(bytes,token));});}
std::shared_ptr<const ImageWriter> writerAdapter(){
    return sectorWriterAdapter(std::string(Constants::WriterId),{std::string(Constants::FormatId)},{std::string(Constants::Extension)},
        [](const auto& image,const auto&,auto,auto){return canWrite(image);},
        [](const auto& image,const auto&,const std::filesystem::path& output,auto,std::stop_token token){
            write(image,output,token);return std::vector<std::filesystem::path>{output};
        });
}
}
