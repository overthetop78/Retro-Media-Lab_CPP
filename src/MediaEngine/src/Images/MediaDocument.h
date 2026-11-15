#pragma once
#include "MediaSource.h"
#include <RetroMediaLab/MediaEngine/MediaKind.h>
#include <RetroMediaLab/MediaEngine/VolumeInfo.h>
#include <RetroMediaLab/MediaEngine/RepresentationInfo.h>
namespace RetroMediaLab::MediaEngine {
class MediaDocument final:public MediaFileSystems::ImageDocument {
public:
    RETROMEDIALAB_MEDIAENGINE_API MediaDocument(std::shared_ptr<const MediaSource>,std::string formatId,MediaKind,std::shared_ptr<const MediaFileSystems::ImageRepresentation>,std::vector<MediaFileSystems::VolumeDescriptor> volumes={},std::vector<MediaFileSystems::MessageRecipe> messages={},std::map<std::string,std::string> metadata={},std::vector<std::string> diagnostics={});
    const MediaFileSystems::SourceDescriptor& source()const override{return *source_;}
    const std::shared_ptr<const MediaSource>& mediaSource()const noexcept{return source_;}
    const std::string& formatId()const override{return format_;}
    MediaKind mediaKind()const noexcept{return kind_;}
    bool isTape()const override{return kind_==MediaKind::Tape;}
    bool isHardDisk()const override{return kind_==MediaKind::HardDisk;}
    const MediaFileSystems::ImageRepresentation& representation()const override{return *representation_;}
    const std::shared_ptr<const MediaFileSystems::ImageRepresentation>& representationOwner()const noexcept{return representation_;}
    const RepresentationInfo& representationInfo()const{return dynamic_cast<const RepresentationInfo&>(*representation_);}
    std::span<const MediaFileSystems::VolumeDescriptor> volumes()const override{return volumes_;}
    std::span<const VolumeInfo> volumeInfos()const noexcept{return publicVolumes_;}
    std::span<const std::string> diagnostics()const override{return diagnostics_;}
    std::span<const MediaFileSystems::MessageRecipe> messages()const override{return messages_;}
    std::span<const MessageInfo> messageInfos()const noexcept{return publicMessages_;}
    const std::map<std::string,std::string>& metadata()const override{return metadata_;}
    RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaDocument> withVolumes(std::vector<MediaFileSystems::VolumeDescriptor>)const;
private:
    std::shared_ptr<const MediaSource> source_;std::string format_;MediaKind kind_;
    std::shared_ptr<const MediaFileSystems::ImageRepresentation> representation_;
    std::vector<MediaFileSystems::VolumeDescriptor> volumes_;std::vector<VolumeInfo> publicVolumes_;
    std::vector<MediaFileSystems::MessageRecipe> messages_;std::vector<MessageInfo> publicMessages_;
    std::map<std::string,std::string> metadata_;std::vector<std::string> diagnostics_;
};
}
