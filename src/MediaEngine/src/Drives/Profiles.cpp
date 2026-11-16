#include <RetroMediaLab/MediaEngine/DriveService.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include "../Audio/Mechanical/RecordedSounds.h"
#include "../Audio/Mechanical/Constants.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
std::vector<std::string> MediaDriveService::soundProfiles(MediaKind kind){
    namespace C=MechanicalAudio::Constants;
    std::string_view device;
    switch(kind){case MediaKind::Tape:device=C::Tape;break;case MediaKind::Floppy:device=C::Floppy;break;case MediaKind::HardDisk:device=C::HardDisk;break;case MediaKind::Optical:device=C::Optical;break;default:throw MediaError(DiagnosticCode::MechanicalRequest);}
    std::vector<std::string> result;
    for(const auto* profile:MechanicalAudio::recordedProfilesForDevice(device))result.push_back(profile->id);
    const auto append=[&](std::string_view id){if(std::find(result.begin(),result.end(),id)==result.end())result.emplace_back(id);};
    switch(kind){
    case MediaKind::Tape:append(C::DefaultTape);break;
    case MediaKind::Floppy:append(C::Floppy525);append(C::DefaultFloppy);break;
    case MediaKind::HardDisk:append(C::Winchester);append(C::DefaultHardDisk);append(C::HardDisk25);break;
    case MediaKind::Optical:append(C::DefaultOptical);append(C::OpticalSlot);break;
    default:break;
    }
    return result;
}
std::string MediaDriveService::soundProfileDisplayName(std::string_view id){
    if(const auto* profile=MechanicalAudio::findRecordedProfile(id))return profile->displayName;
    if(id==MechanicalAudio::Constants::HardDisk25)return std::string(MechanicalAudio::Constants::Label25);
    return std::string(id);
}
}
