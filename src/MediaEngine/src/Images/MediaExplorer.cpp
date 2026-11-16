#include "MediaExplorer.h"
#include "ExplorationMapper.h"
#include "SequentialContentAdapter.h"
#include "../Composition/SequentialMedia.h"
#include "../Sources/DriveOperation.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
MediaExplorer MediaExplorer::createDefault(){
    const auto sequential=SequentialMediaComposition::createDefault();
    return MediaExplorer(MediaFileSystems::MediaExplorer::createDefault({std::make_shared<SequentialContentAdapter>(sequential.decoders())}));
}
std::future<ExploredMediaImage> MediaExplorer::exploreAsync(std::shared_ptr<const MediaDocument> document,std::stop_token token)const{
    return std::async(std::launch::deferred,[explorer=explorer_,document=std::move(document),token]{
        if(!document)throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::DocumentNull);
        const auto drive=document->mediaSource()->options().drive;DriveOperationScope operation(drive.get());
        const auto result=explorer.exploreAsync(document,token).get();ExploredMediaImage output;
        output.document=std::equal(document->volumes().begin(),document->volumes().end(),result.volumes.begin(),result.volumes.end())?document:document->withVolumes(result.volumes);
        output.sourcePath=document->source().primaryPath();output.formatId=document->formatId();output.mediaKind=document->mediaKind();output.representationKind=document->representationInfo().representationKind();output.metadata=document->metadata();output.diagnostics.assign(document->diagnostics().begin(),document->diagnostics().end());output.messages.assign(document->messageInfos().begin(),document->messageInfos().end());output.firstFileName=result.firstFileName;
        for(const auto& error:result.diagnostics)output.explorationDiagnostics.push_back(ExplorationMapper::diagnostic(error));
        for(const auto& volume:result.exploredVolumes){ExploredVolumeInfo info;info.descriptor=ExplorationMapper::volumeDescriptor(volume.descriptor);info.readerId=volume.readerId;if(volume.fileSystem)info.fileSystem=ExplorationMapper::volume(volume.fileSystem,volume.analyzedEntries);for(const auto& error:volume.diagnostics)info.diagnostics.push_back(ExplorationMapper::diagnostic(error));output.volumes.push_back(std::move(info));}
        operation.finish();return output;
    });
}
}
