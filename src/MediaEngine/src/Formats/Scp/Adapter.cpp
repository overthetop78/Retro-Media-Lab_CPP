#include "Adapter.h"
#include "ProtectedImage.h"
#include "Writer.h"
#include "../Definitions/DiskDefinitions.h"
#include "../../Images/WriterAdapters.h"
#include "../../Images/DocumentFactory.h"
#include "../../Sources/RandomAccessSources.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Scp {
const ReaderCapabilities& Reader::capabilities()const{
    static const ReaderCapabilities result{{std::string(Constants::FormatId)},{std::string(Constants::Extension)},{{Constants::Signature.begin(),Constants::Signature.end()}},{},{MediaKind::Floppy},{RepresentationKind::Flux}};
    return result;
}
bool Reader::supportsFormatId(std::string_view id)const{return Primitives::ordinalIdentifierEqualsIgnoreCase(id,Constants::FormatId);}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{
    return std::async(std::launch::deferred,[context,token]{
        checkCancellation(token);if(context.requestedFormatId()&&!Primitives::ordinalIdentifierEqualsIgnoreCase(*context.requestedFormatId(),Constants::FormatId))return false;
        if(context.length()<Constants::HeaderBytes)return false;
        return hasSignature(context.readHeaderAsync(std::int32_t(Constants::Signature.size()),token).get());
    });
}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{
    return std::async(std::launch::deferred,[cache=cache_,context,token]{
        const auto bytes=context.readBytesAsync(token).get();checkCancellation(token);
        auto image=std::make_shared<const Image>(read(bytes,token));const auto& sourcePath=context.source()->primaryPath();
        cache.remember(std::filesystem::path(std::u8string(sourcePath.begin(),sourcePath.end())),image);
        return floppyFluxDocument(context.source(),std::string(Constants::FormatId),std::make_shared<const Flux::ProtectedTrackImage>(protectedImage(image,token)),metadata(*image));
    });
}
std::future<SharedImage> Reader::readFileAsync(std::filesystem::path path,std::stop_token token)const{return cache_.getOrAddAsync(std::move(path),[](const auto& source,std::stop_token lifetime){return std::make_shared<const Image>(read(*fileSource(source),lifetime));},token);}
void Reader::remember(const std::filesystem::path& path,SharedImage image)const{cache_.remember(path,std::move(image));}
std::shared_ptr<const ImageWriter> writerAdapter(){
    auto formats=Definitions::bitcellFormatIds();formats.emplace_back(Constants::FormatId);
    return fluxWriterAdapter(std::string(Constants::WriterId),std::move(formats),{std::string(Constants::Extension)},
        [](const auto&,auto,auto){return true;},
        [](const auto& image,const auto& metadata,const std::filesystem::path& path,auto,std::stop_token token){write(fromProtected(image,metadata,token),path,token);return std::vector<std::filesystem::path>{path};});
}
}
