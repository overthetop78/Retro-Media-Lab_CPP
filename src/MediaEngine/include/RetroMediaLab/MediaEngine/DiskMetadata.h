#pragma once
#include <optional>
#include <string>
#include <vector>
#include <cstdint>
namespace RetroMediaLab::MediaEngine {
struct DiskContentMetadata {
 bool hasValidAmigaBootLoader=false;
 std::optional<std::string> modificationId;
 std::vector<std::string> compressionIds;
 std::optional<std::string> organizationId;
 std::int32_t organizationMemberCount=0;
 bool hasIdentifiedCharacteristics()const noexcept{return modificationId.has_value()||!compressionIds.empty()||organizationId.has_value();}
 bool hasCataloglessOrganization()const noexcept{return organizationId.has_value()||(hasValidAmigaBootLoader&&hasIdentifiedCharacteristics());}
};
struct DiskImageMetadata {
 std::vector<std::string> systemIds;
 std::optional<std::string> protectionId;
 DiskContentMetadata content;
};
}
