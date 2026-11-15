#include "MediaDocument.h"
#include "ExplorationMapper.h"
#include "../Models/Blocks/BlockImage.h"
#include "../Models/Optical/Image.h"
#include "../Models/Sectors/Representation.h"
#include <QString>
namespace RetroMediaLab::MediaEngine {
namespace M=MediaFileSystems;
MediaDocument::MediaDocument(std::shared_ptr<const MediaSource> source,std::string format,MediaKind kind,std::shared_ptr<const M::ImageRepresentation> representation,std::vector<M::VolumeDescriptor> volumes,std::vector<M::MessageRecipe> messages,std::map<std::string,std::string> metadata,std::vector<std::string> diagnostics):source_(std::move(source)),format_(std::move(format)),kind_(kind),representation_(std::move(representation)),volumes_(std::move(volumes)),messages_(std::move(messages)),metadata_(std::move(metadata)),diagnostics_(std::move(diagnostics)){
    if(!source_)throw M::MediaError(M::DiagnosticCode::ContextSourceNull);
    if(QString::fromUtf8(format_.data(),qsizetype(format_.size())).trimmed().isEmpty())throw M::MediaError(M::DiagnosticCode::DocumentFormatId);
    if(!representation_||!dynamic_cast<const RepresentationInfo*>(representation_.get()))throw M::MediaError(M::DiagnosticCode::DocumentRepresentation);
    if(source_->options().trackDriveAccesses&&source_->options().drive){
        if(const auto blocks=std::dynamic_pointer_cast<const BlockImage>(representation_))representation_=bindBlocks(blocks,source_->options().drive,source_->options().driveWriting);
        else if(const auto optical=std::dynamic_pointer_cast<const OpticalImage>(representation_))representation_=bindOptical(optical,source_->options().drive,source_->options().driveWriting);
        else if(const auto sectors=std::dynamic_pointer_cast<const SectorRepresentation>(representation_))representation_=bindSectors(sectors,source_->options().drive,source_->options().driveWriting);
    }
    publicVolumes_.reserve(volumes_.size());for(const auto& value:volumes_)publicVolumes_.push_back(ExplorationMapper::volumeDescriptor(value));
    publicMessages_.reserve(messages_.size());for(const auto& value:messages_)publicMessages_.push_back(ExplorationMapper::message(value));
}
std::shared_ptr<const MediaDocument> MediaDocument::withVolumes(std::vector<M::VolumeDescriptor> volumes)const{return std::make_shared<MediaDocument>(source_,format_,kind_,representation_,std::move(volumes),messages_,metadata_,diagnostics_);}
}
