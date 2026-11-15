#include "Adapter.h"
#include "Constants.h"
#include "../../Images/DocumentFactory.h"
#include "../../Sources/RandomAccessSources.h"
#include "AdfImages.h"
#include "../../Models/Sectors/Representation.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Adf {
const ReaderCapabilities& Reader::capabilities()const {
    static const auto result=[]{ReaderCapabilities c;for(const auto& p:acornGeometries())c.formatIds.push_back(p.geometry.formatId);for(const auto& p:amigaGeometries())c.formatIds.push_back(p.geometry.formatId);for(auto extension:Constants::Extensions)c.extensions.emplace_back(extension);c.mediaKinds={MediaKind::Floppy};c.representationKinds={RepresentationKind::Sectors};return c;}();return result;
}
bool Reader::supportsFormatId(std::string_view id)const{return findGeometry(id).has_value();}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token]{const auto requested=context.requestedFormatId();return canRead(context.dataSource(),context.extension(),requested?std::optional<std::string_view>(*requested):std::nullopt,token);});
}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const {
    return std::async(std::launch::deferred,[context,token]{
        const auto bytes=context.readBytesAsync(token).get();const auto requested=context.requestedFormatId();const auto image=read(memorySource(bytes),requested?std::optional<std::string_view>(*requested):std::nullopt,token);
        if(bytes.size()<image->capacity())return std::make_shared<const MediaDocument>(context.source(),image->geometry().formatId,MediaKind::Floppy,std::make_shared<SectorRepresentation>(image),std::vector<MediaFileSystems::VolumeDescriptor>{},std::vector<MediaFileSystems::MessageRecipe>{{std::string(diagnosticKey(DiagnosticCode::AdfInvalidLength)),{std::to_string(bytes.size()),std::to_string(image->capacity())},std::string(Constants::TranslationDomain)}});
        return floppySectorDocument(context.source(),image);
    });
}
}
