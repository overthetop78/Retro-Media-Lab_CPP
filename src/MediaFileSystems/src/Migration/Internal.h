#pragma once
#include <RetroMediaLab/MediaFileSystems/Migration.h>
namespace RetroMediaLab::MediaFileSystems::MigrationInternal {
MigrationTargetCapabilities capabilities(const MigrationPlan&, const SectorImage&);
}
