#pragma once
#include <RetroMediaLab/MediaEngine/ExplorationInfo.h>
#include <RetroMediaLab/MediaEngine/Export.h>
#include <RetroMediaLab/MediaFileSystems/FileSystemModels.h>
#include <RetroMediaLab/MediaFileSystems/Volumes.h>
namespace RetroMediaLab::MediaEngine::ExplorationMapper {
RETROMEDIALAB_MEDIAENGINE_API MessageInfo message(const MediaFileSystems::MessageRecipe&);
RETROMEDIALAB_MEDIAENGINE_API VolumeInfo volumeDescriptor(const MediaFileSystems::VolumeDescriptor&);
RETROMEDIALAB_MEDIAENGINE_API DiagnosticInfo diagnostic(const MediaFileSystems::Diagnostic&);
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const FileSystemVolumeInfo> volume(std::shared_ptr<const MediaFileSystems::FileSystemVolume>,std::optional<std::span<const MediaFileSystems::AnalyzedFileSystemEntry>> analyzed={});
RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const FileSystemVolumeInfo> volumeForDisk(std::shared_ptr<const MediaFileSystems::FileSystemVolume>,std::optional<std::span<const MediaFileSystems::AnalyzedFileSystemEntry>> analyzed={});
RETROMEDIALAB_MEDIAENGINE_API MediaFileSystems::FileSystemVolume toFileSystemsVolume(const FileSystemVolumeInfo&);
}
