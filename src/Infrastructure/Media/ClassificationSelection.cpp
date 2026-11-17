#include "ClassificationSelection.h"
#include <algorithm>
namespace RetroMediaLab::Infrastructure {
ClassificationSelection::ClassificationSelection():catalog_(MediaEngine::DiskClassificationCatalog::createDefault()){}
void ClassificationSelection::normalizeProtection(){const auto choices=catalog_.protectionsFor(machine_,format_);if(!protection_||std::none_of(choices.begin(),choices.end(),[this](const auto& value){return value.id==*protection_;}))protection_.reset();}
void ClassificationSelection::selectMachine(std::optional<std::string> machine){
    if(machine&&std::find(catalog_.machines().begin(),catalog_.machines().end(),*machine)==catalog_.machines().end())return;
    machine_=std::move(machine);const auto formats=catalog_.formatsFor(machine_);
    const auto detected=std::find_if(formats.begin(),formats.end(),[this](const auto& value){return std::find(detected_.begin(),detected_.end(),value.id)!=detected_.end();});
    const auto common=std::find_if(formats.begin(),formats.end(),[](const auto& value){return value.isCommon;});
    const auto chosen=detected!=formats.end()?detected:common!=formats.end()?common:formats.begin();format_=chosen==formats.end()?std::nullopt:std::optional<std::string>(chosen->id);normalizeProtection();
}
void ClassificationSelection::selectFormat(std::optional<std::string> id){
    if(const auto* format=catalog_.resolveFormat(id)){machine_=format->family;format_=format->id;}else format_.reset();normalizeProtection();
}
void ClassificationSelection::selectProtection(std::optional<std::string> id){protection_=std::move(id);normalizeProtection();}
void ClassificationSelection::setAutomatic(bool enabled){automatic_=enabled;if(enabled){selectFormat(detectedFormat_);if(!format_)machine_.reset();selectProtection(detectedProtection_);}}
void ClassificationSelection::detect(const MediaEngine::DiskImageInfo& disk){
    detected_=disk.detectedFormatIds;detectedFormat_=disk.primaryFormatId.empty()?std::nullopt:std::optional<std::string>(disk.primaryFormatId);detectedProtection_=disk.metadata.protectionId;
    if(automatic_){selectFormat(detectedFormat_);if(!format_)for(const auto& id:detected_)if(catalog_.resolveFormat(id)){selectFormat(id);break;}if(!format_)machine_.reset();selectProtection(detectedProtection_);}
}
MediaEngine::DiskVisualizationClassification ClassificationSelection::visualization()const{return MediaEngine::classifyDiskVisualization(machine_,format_,protection_,automatic_);}
}
