#pragma once
#include "ProDos.h"
namespace RetroMediaLab::MediaFileSystems::AppleSos {
RETROMEDIALAB_MEDIAFILESYSTEMS_API SectorWritePlan write(const MigrationPlan&, const SectorGeometry& proDosGeometry, std::uint32_t logicalBlockCount, std::span<const SectorAddress>, std::string sosFormatId, std::optional<Timestamp> defaultTime = {}, std::stop_token = {});
}
