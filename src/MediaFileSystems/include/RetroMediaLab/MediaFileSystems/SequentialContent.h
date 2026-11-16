#pragma once
#include "MediaDocument.h"
#include "FileSystemModels.h"
namespace RetroMediaLab::MediaFileSystems {
// Owned decoded bytes and metadata; this contract contains no media decoder.
class SequentialDecodedBlock final {
public:
    RETROMEDIALAB_MEDIAFILESYSTEMS_API SequentialDecodedBlock(std::int64_t position,std::vector<std::uint8_t> data,std::optional<bool> integrityValid,std::map<std::string,std::string> metadata);
    std::int64_t position()const noexcept{return position_;}
    std::span<const std::uint8_t> data()const noexcept{return data_;}
    std::optional<bool> integrityValid()const noexcept{return integrity_;}
    const std::map<std::string,std::string>& metadata()const noexcept{return metadata_;}
private:
    std::int64_t position_;std::vector<std::uint8_t> data_;std::optional<bool> integrity_;std::map<std::string,std::string> metadata_;
};
class SequentialContent {
public:
    virtual ~SequentialContent()=default;
    virtual std::optional<std::string> decoderId()const=0;
    virtual std::span<const SequentialDecodedBlock> blocks()const=0;
    virtual std::span<const std::string> diagnostics()const=0;
    virtual std::span<const MessageRecipe> messages()const{return {};}
};
class SequentialContentReader final {
public:
    RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string_view id()const noexcept;
    RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemVolume read(const ImageDocument&,const VolumeDescriptor&,const SequentialContent&,std::string volumeName,std::stop_token={})const;
};
}
