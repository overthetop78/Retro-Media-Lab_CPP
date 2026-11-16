#pragma once
#include "Export.h"
#include "FileSystemModels.h"
#include "SectorImage.h"
namespace RetroMediaLab::MediaFileSystems {
// Canonical fallback plus a language-independent translation recipe; no current-language state.
RETROMEDIALAB_MEDIAFILESYSTEMS_API void applyFileSystemDisplayName(FileSystemVolume&,const SectorGeometry&,std::optional<std::uint16_t> lisaVersion={});
}
