#include "ReadingService.h"
#include "ReadRejection.h"
#include "../Formats/Tape/Raw/Reader.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace {
ReaderRegistry automaticRegistry(const ReaderRegistry& source) {
    std::vector<std::shared_ptr<const ImageReader>> readers;
    for(const auto& reader:source.readers())if(!dynamic_cast<const RawTape::Reader*>(reader.get()))readers.push_back(reader);
    return ReaderRegistry(std::move(readers));
}
bool canFallback(const RecognitionError& error) {
    if(error.diagnostic().code==DiagnosticCode::ReaderNoneAccepted)return true;
    return error.diagnostic().code==DiagnosticCode::ReaderAllRejected&&!error.failures().empty()
        &&std::all_of(error.failures().begin(),error.failures().end(),[](const auto& failure){return isImageReadRejection(failure.exception);});
}
std::shared_ptr<const MediaDocument> attach(std::shared_ptr<const MediaDocument> document,const DriveFactory& factory) {
    if(document->mediaSource()->options().drive||!factory)return document;
    const auto drive=factory(document->mediaKind());if(!drive)return document;
    try {drive->insert();}
    catch(...) {try{drive->reset();}catch(...){}throw;}
    try {
        return std::make_shared<MediaDocument>(document->mediaSource()->withDrive(drive),document->formatId(),document->mediaKind(),document->representationOwner(),
            std::vector<MediaFileSystems::VolumeDescriptor>(document->volumes().begin(),document->volumes().end()),
            std::vector<MediaFileSystems::MessageRecipe>(document->messages().begin(),document->messages().end()),document->metadata(),
            std::vector<std::string>(document->diagnostics().begin(),document->diagnostics().end()));
    } catch(...) {try{drive->reset();}catch(...){}throw;}
}
}
MediaImageReadingService::MediaImageReadingService(ReaderRegistry registry,DriveFactory drives)
    :registry_(std::move(registry)),automaticRegistry_(automaticRegistry(registry_)),drives_(std::move(drives)){}
std::future<std::shared_ptr<const MediaDocument>> MediaImageReadingService::readAsync(std::shared_ptr<const MediaSource> source,std::stop_token token) const {
    return std::async(std::launch::async,[registry=registry_,automatic=automaticRegistry_,factory=drives_,source=std::move(source),token]{
        checkCancellation(token);
        RecognitionContext context(source,ImageConstants::PageSize,{},factory);
        const auto requested=source->options().requestedFormatId;
        const auto& selected=requested&&sameIdentifier(*requested,ImageConstants::RawTapeFormat)?registry:automatic;
        std::shared_ptr<const MediaDocument> document;
        try {document=selected.recognizeAsync(context,token).get().document();}
        catch(const RecognitionError& error) {
            if(requested||context.extension()!=ImageConstants::RawExtension||!canFallback(error))throw;
            checkCancellation(token);
            auto options=source->options();options.requestedFormatId=std::string(ImageConstants::RawTapeFormat);
            return RawTape::Reader{}.readAsync(RecognitionContext(std::make_shared<MediaSource>(std::move(options))),token).get();
        }
        return attach(std::move(document),factory);
    });
}
}
