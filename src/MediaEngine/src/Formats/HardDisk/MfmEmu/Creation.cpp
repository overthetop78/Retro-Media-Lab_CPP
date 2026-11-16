#include "Adapter.h"
#include "Representation.h"
#include "Constants.h"
#include "../Acorn/Descriptor.h"
#include "../Adaptec/Constants.h"
#include <RetroMediaLab/MediaFileSystems/Adfs.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::MfmEmu {
std::future<void> createAsync(std::filesystem::path path,HardDiskGeometry geometry,std::optional<std::string> fileSystemId,std::stop_token token,bool preallocate){return std::async(std::launch::deferred,[path=std::move(path),geometry,fileSystemId=std::move(fileSystemId),token,preallocate]{checkCancellation(token);if(!AcornWinchester::supportsGeometry(geometry)||geometry.cylinders()>Adaptec::Constants::MaximumCylinders||geometry.heads()>Adaptec::Constants::MaximumHeads)throw MediaError(DiagnosticCode::MfmCreation);if(fileSystemId&&(*fileSystemId!=Constants::AdfsId||geometry.sectorSize()!=Adaptec::Constants::MinimumBlockSize))throw MediaError(DiagnosticCode::MfmCreationFileSystem);auto header=Header::create(std::int32_t(geometry.cylinders()),geometry.heads(),Constants::AdaptecTrackByteLength,Constants::AdaptecSampleRate);auto container=fileSystemId?Container::formatted(std::move(header),geometry,MediaFileSystems::Adfs::createBbcHeader(std::uint32_t(geometry.logicalSectorCount()))):Container::blank(std::move(header));SourceOptions options;const auto name=path.u8string();options.primaryPath={name.begin(),name.end()};auto document=std::make_shared<const MediaDocument>(std::make_shared<const MediaSource>(std::move(options)),std::string(Constants::FormatId),MediaKind::HardDisk,std::make_shared<const Representation>(std::move(container)));Writer{}.createAsync(std::move(document),path,token,preallocate).get();});}
}
