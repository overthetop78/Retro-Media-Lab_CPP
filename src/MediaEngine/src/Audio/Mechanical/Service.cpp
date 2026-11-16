#include <RetroMediaLab/MediaEngine/MechanicalSound.h>
#include <RetroMediaLab/MediaEngine/DriveService.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include "RecordedBank.h"
namespace RetroMediaLab::MediaEngine {
namespace {
std::string_view device(MediaKind kind){namespace C=MechanicalAudio::Constants;switch(kind){case MediaKind::Tape:return C::Tape;case MediaKind::Floppy:return C::Floppy;case MediaKind::HardDisk:return C::HardDisk;case MediaKind::Optical:return C::Optical;default:throw MediaError(DiagnosticCode::MechanicalRequest);}}
std::string_view profile(std::string_view name,const std::optional<std::string>& requested){if(requested&&MechanicalAudio::findRecordedProfile(*requested))return *requested;const auto base=MechanicalAudio::baseRecordedProfile(name);if(!base)throw MediaError(DiagnosticCode::MechanicalRequest);return *base;}
}
struct MechanicalSoundService::State {std::shared_ptr<MechanicalAudio::RecordedRenderer> bank;};
MechanicalSoundService::MechanicalSoundService(std::filesystem::path root,std::filesystem::path executable):state_(std::make_shared<State>()){
    state_->bank=root.empty()&&executable.empty()?MechanicalAudio::sharedRecordedBank():std::shared_ptr<MechanicalAudio::RecordedRenderer>(MechanicalAudio::createRecordedBank(std::move(root),std::move(executable)));
}
MechanicalSoundService::~MechanicalSoundService()=default;
std::vector<std::string> MechanicalSoundService::profiles(MediaKind kind){return MediaDriveService::soundProfiles(kind);}
std::string MechanicalSoundService::profileDisplayName(std::string_view id){return MediaDriveService::soundProfileDisplayName(id);}
std::vector<std::int16_t> MechanicalSoundService::render(MediaKind kind,std::string_view operation,std::int32_t rate,std::int32_t count,std::int64_t offset,std::optional<std::string> requested,double volume,std::int32_t distance,std::int32_t direction,std::int32_t cylinder)const{
    const auto name=device(kind);return state_->bank->render(name,operation,rate,count,offset,profile(name,requested),volume,distance,direction,cylinder);
}
double MechanicalSoundService::duration(MediaKind kind,std::string_view operation,std::optional<std::string> requested,std::int32_t distance,std::int32_t direction,std::int32_t cylinder)const{const auto name=device(kind);return state_->bank->duration(profile(name,requested),operation,distance,direction,cylinder);}
void MechanicalSoundService::prepareProfile(MediaKind kind,std::int32_t rate,std::optional<std::string> requested)const{const auto name=device(kind);state_->bank->prepareProfile(profile(name,requested),rate);}
}
