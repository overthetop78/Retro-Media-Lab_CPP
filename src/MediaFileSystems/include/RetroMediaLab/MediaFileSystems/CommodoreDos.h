#pragma once
#include "FileSystemModels.h"
#include "SectorImage.h"
#include "Migration.h"
namespace RetroMediaLab::MediaFileSystems::CommodoreDos {
class RETROMEDIALAB_MEDIAFILESYSTEMS_API WritePolicy {
public:
    WritePolicy();
    WritePolicy(std::uint8_t defaultFileType,std::uint8_t relativeRecordLength);
    std::uint8_t defaultFileType() const noexcept {return type_;}
    std::uint8_t relativeRecordLength() const noexcept {return recordLength_;}
private:
    std::uint8_t type_,recordLength_;
};
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool validName(std::string_view);
RETROMEDIALAB_MEDIAFILESYSTEMS_API SectorWritePlan write(const MigrationPlan&,std::string_view formatId,const WritePolicy& = {},std::stop_token = {});
RETROMEDIALAB_MEDIAFILESYSTEMS_API bool canRead(const SectorImage&,std::stop_token={});
RETROMEDIALAB_MEDIAFILESYSTEMS_API FileSystemVolume read(const SectorImage&,std::stop_token={});
RETROMEDIALAB_MEDIAFILESYSTEMS_API std::string_view baseTypeName(std::uint8_t) noexcept;
}
