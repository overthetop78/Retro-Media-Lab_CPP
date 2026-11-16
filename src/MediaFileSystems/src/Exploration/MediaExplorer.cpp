#include <RetroMediaLab/MediaFileSystems/MediaExplorer.h>
#include <RetroMediaLab/MediaFileSystems/FileTreeCursor.h>
#include "Composition/Internal.h"
#include <RetroMediaLab/MediaFileSystems/SectorRegistry.h>
#include <RetroMediaLab/MediaFileSystems/CartridgeReaders.h>
#include <RetroMediaLab/MediaFileSystems/FamicomDisk.h>
#include <RetroMediaLab/MediaFileSystems/GameCubeFst.h>
#include <RetroMediaLab/MediaFileSystems/OpticalReaders.h>
#include <algorithm>
#include <map>
namespace RetroMediaLab::MediaFileSystems {
struct MediaExplorer::Impl {
    std::vector<std::shared_ptr<const MediaFileSystemReader>> readers;
    VolumeDetectorRegistry detectors;
};
namespace {
ExploredFileSystemVolume exploreVolume(const ImageDocument& document,const VolumeDescriptor& volume,std::vector<std::shared_ptr<const MediaFileSystemReader>> readers,std::stop_token token) {
    if(volume.fileSystemId()&&!Composition::blank(*volume.fileSystemId())) {
        const auto selected=Composition::ordinalKey(*volume.fileSystemId());
        std::stable_partition(readers.begin(),readers.end(),[&](const auto& reader){return Composition::ordinalKey(reader->id())==selected;});
    }
    ExploredFileSystemVolume result{volume,{},{},{},{}};
    for(const auto& reader:readers) {
        if(!reader->canRead(document,volume,token))continue;
        try{result.fileSystem=std::make_shared<FileSystemVolume>(reader->read(document,volume,token));result.readerId=reader->id();return result;}
        catch(const MediaError& error){if(error.kind()!=ErrorKind::InvalidData)throw;auto diagnostic=error.diagnostic();diagnostic.context.insert(diagnostic.context.begin(),std::string(reader->id()));result.diagnostics.push_back(std::move(diagnostic));}
    }
    return result;
}
std::optional<std::string> firstFile(const std::vector<FileSystemEntry>& entries) {
    for(const auto& entry:entries){FileTreeCursor cursor(&entry,[](const void* value){return static_cast<const FileSystemEntry*>(value)->kind==EntryKind::File;},[](const void* value){const auto& children=static_cast<const FileSystemEntry*>(value)->children;return FileChildrenView{children.data(),children.size(),[](const void* data,std::size_t index)->const void*{return static_cast<const FileSystemEntry*>(data)+index;}};});if(const auto first=cursor.next())return static_cast<const FileSystemEntry*>(first)->name;}
    return {};
}
}
MediaExplorer::MediaExplorer(std::vector<std::shared_ptr<const MediaFileSystemReader>> readers,VolumeDetectorRegistry detectors) {
    for(std::size_t index=0;index<readers.size();++index){if(!readers[index])throw MediaError(DiagnosticCode::RegistryNullReader,{std::to_string(index)});if(Composition::blank(readers[index]->id()))throw MediaError(DiagnosticCode::RegistryEmptyReaderId,{std::to_string(index)});}
    std::map<std::u16string,std::size_t> counts;for(const auto& reader:readers)++counts[Composition::ordinalKey(reader->id())];
    for(const auto& reader:readers)if(counts.at(Composition::ordinalKey(reader->id()))>1)throw MediaError(DiagnosticCode::RegistryDuplicateReaderId,{std::string(reader->id())});
    impl_=std::make_shared<Impl>(Impl{std::move(readers),std::move(detectors)});
}
std::span<const std::shared_ptr<const MediaFileSystemReader>> MediaExplorer::readers()const noexcept{return impl_->readers;}
std::future<MediaFileSystemExplorationResult> MediaExplorer::exploreAsync(std::shared_ptr<const ImageDocument> document,std::stop_token token)const {
    return std::async(std::launch::async,[document=std::move(document),impl=impl_,token] {
        if(!document)throw MediaError(DiagnosticCode::DocumentNull);
        auto detected=impl->detectors.detectAsync(document,token).get();
        MediaFileSystemExplorationResult result{std::move(detected.volumes),{},std::move(detected.diagnostics),{}};
        for(const auto& descriptor:result.volumes) {
            checkCancellation(token);auto volume=exploreVolume(*document,descriptor,impl->readers,token);
            if(volume.fileSystem)volume.analyzedEntries=analyzeEntryViews(document->formatId(),volume.fileSystem,token);
            result.exploredVolumes.push_back(std::move(volume));
        }
        for(const auto& volume:result.exploredVolumes)if(volume.fileSystem)if(auto name=firstFile(volume.fileSystem->entries)){result.firstFileName=std::move(name);break;}
        return result;
    });
}
MediaExplorer MediaExplorer::createDefault(std::vector<std::shared_ptr<const MediaFileSystemReader>> adapters) {
    const SectorRegistry sectors;
    std::vector<std::shared_ptr<const MediaFileSystemReader>> readers;
    for(const auto& reader:sectors.readers())readers.push_back(reader);
    for(auto& reader:adapters)readers.push_back(std::move(reader));
    readers.push_back(std::make_shared<AmstradCartridgeReader>());
    readers.push_back(std::make_shared<ConsoleCartridgeReader>());
    readers.push_back(std::make_shared<FamicomDiskReader>());
    readers.push_back(std::make_shared<GameCubeFstReader>());
    readers.push_back(std::make_shared<XdvdfsReader>());
    readers.push_back(std::make_shared<UdfReader>());
    readers.push_back(std::make_shared<JolietReader>());
    readers.push_back(std::make_shared<RockRidgeReader>());
    readers.push_back(std::make_shared<Iso9660Reader>());
    return MediaExplorer(std::move(readers),VolumeDetectorRegistry(createDefaultVolumeDetectors()));
}
}
