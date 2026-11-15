#include "DocumentFactory.h"
#include "../Models/Sectors/Representation.h"
#include "../Models/Flux/Representation.h"
namespace RetroMediaLab::MediaEngine {
namespace M=MediaFileSystems;
std::shared_ptr<const MediaDocument> floppySectorDocument(std::shared_ptr<const MediaSource> source,std::shared_ptr<const M::SectorImage> image,std::map<std::string,std::string> metadata){
    if(!image)throw M::MediaError(M::DiagnosticCode::RegistryNullImage);
    const auto format=image->geometry().formatId;
    return std::make_shared<MediaDocument>(std::move(source),format,MediaKind::Floppy,std::make_shared<SectorRepresentation>(std::move(image)),std::vector<M::VolumeDescriptor>{},std::vector<M::MessageRecipe>{},std::move(metadata));
}
std::shared_ptr<const MediaDocument> floppyFluxDocument(std::shared_ptr<const MediaSource> source,std::string format,std::shared_ptr<const Flux::ProtectedTrackImage> image,std::map<std::string,std::string> metadata){return std::make_shared<MediaDocument>(std::move(source),std::move(format),MediaKind::Floppy,std::make_shared<FluxRepresentation>(std::move(image)),std::vector<M::VolumeDescriptor>{},std::vector<M::MessageRecipe>{},std::move(metadata));}
}
