#include "WritingService.h"
#include "../Sources/DriveOperation.h"
#include "../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
MediaImageWritingService::MediaImageWritingService(WriterRegistry writers,MediaDriveService drives):writers_(std::move(writers)),drives_(std::move(drives)){}
std::future<std::vector<std::filesystem::path>> MediaImageWritingService::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path output,std::string format,std::stop_token token)const{
    return std::async(std::launch::async,[writers=writers_,drives=drives_,document=std::move(document),output=std::move(output),format=std::move(format),token]()mutable{
        checkCancellation(token);
        if(!document||output.empty()||Primitives::isBlankIdentifier(format))throw MediaError(DiagnosticCode::ImageWriteRequest);
        const auto u8=output.extension().u8string();const std::string extension(u8.begin(),u8.end());
        if(Primitives::isBlankIdentifier(extension))throw MediaError(DiagnosticCode::ImageWriteRequest);
        const auto writer=writers.resolve(*document,format,extension);
        if(!writer)throw MediaError(DiagnosticCode::ImageWriteUnavailable,{format,extension});
        DriveOperationScope input(document->mediaSource()->options().drive.get());
        const auto destination=drives.create(document->mediaKind());DriveOperationScope activity(destination.get());
        if(destination){
            destination->insert();auto source=document->mediaSource()->options();source.drive=destination;source.driveWriting=true;
            document=std::make_shared<MediaDocument>(std::make_shared<MediaSource>(std::move(source)),document->formatId(),document->mediaKind(),document->representationOwner(),
                std::vector<MediaFileSystems::VolumeDescriptor>(document->volumes().begin(),document->volumes().end()),
                std::vector<MediaFileSystems::MessageRecipe>(document->messages().begin(),document->messages().end()),document->metadata(),
                std::vector<std::string>(document->diagnostics().begin(),document->diagnostics().end()));
        }
        const auto files=writer->writeAsync(document,output,format,token).get();
        if(files.empty()||std::any_of(files.begin(),files.end(),[](const auto& path){const auto u8=path.u8string();return path.empty()||Primitives::isBlankIdentifier(std::string(u8.begin(),u8.end()));}))throw MediaError(DiagnosticCode::ImageWriteOutputs,{writer->capabilities().id});
        activity.finish();input.finish();return files;
    });
}
}
