#include "RecordedSounds.h"
#include "Constants.h"
#include "CatalogData.h"
#include <algorithm>
#include <cstdlib>
#include <limits>
#include <tuple>
namespace RetroMediaLab::MediaEngine::MechanicalAudio {
std::string_view RecordedSample::id() const noexcept {const std::string_view path=filePath;return path.ends_with(Constants::Flac)?path.substr(0,path.size()-Constants::Flac.size()):path;}
std::span<const RecordedProfile> recordedProfiles(){
    static const auto profiles=[] {
        std::vector<RecordedProfile> result;result.reserve(CatalogData::Profiles.size());
        for(const auto& definition:CatalogData::Profiles){
            RecordedProfile profile{std::string(definition.id),std::string(definition.device),std::string(definition.displayName),{}};
            profile.samples.reserve(definition.sampleCount);
            for(std::size_t index=0;index<definition.sampleCount;++index){
                const auto& source=CatalogData::Samples[definition.firstSample+index];RecordedSample sample{std::string(source.filePath),{}};
                sample.operations.reserve(source.operationCount);
                for(std::size_t operation=0;operation<source.operationCount;++operation){
                    const auto& action=CatalogData::Operations[source.firstOperation+operation];
                    std::optional<std::string> start;if(action.startOperation)start=std::string(*action.startOperation);
                    sample.operations.push_back({std::string(action.operation),action.distance,action.direction,action.cylinder,action.loop,std::move(start)});
                }
                profile.samples.push_back(std::move(sample));
            }
            result.push_back(std::move(profile));
        }
        return result;
    }();
    return profiles;
}
const RecordedProfile* findRecordedProfile(std::string_view id){for(const auto& profile:recordedProfiles())if(profile.id==id)return &profile;return nullptr;}
std::optional<std::string_view> baseRecordedProfile(std::string_view device) noexcept {
    using namespace Constants;
    if(device==Tape)return DefaultTape;if(device==Floppy)return DefaultFloppy;if(device==HardDisk)return DefaultHardDisk;if(device==Optical)return DefaultOptical;return {};
}
std::vector<const RecordedProfile*> recordedProfilesForDevice(std::string_view device){
    const auto fallback=baseRecordedProfile(device);std::vector<const RecordedProfile*> result;
    for(const auto& profile:recordedProfiles())if(profile.device==device&&std::any_of(profile.samples.begin(),profile.samples.end(),[](const auto& sample){return std::any_of(sample.operations.begin(),sample.operations.end(),[](const auto& operation){return operation.operation==Constants::Motor;});}))result.push_back(&profile);
    std::stable_partition(result.begin(),result.end(),[&](const auto* profile){return fallback&&profile->id==*fallback;});
    return result;
}
const RecordedSample* selectRecordedSampleInProfile(const RecordedProfile& profile,std::string_view operation,std::int32_t distance,std::int32_t direction,std::int32_t cylinder){
    using Rank=std::tuple<bool,std::int64_t,std::int64_t>;
    const auto sign=(direction>0)-(direction<0);const auto absolute=std::abs(std::int64_t(distance));
    std::optional<Rank> best;const RecordedSample* selected=nullptr;
    for(const auto& sample:profile.samples)for(const auto& action:sample.operations)if(action.operation==operation){
        const Rank rank{action.direction!=0&&action.direction!=sign,action.distance==0?std::int64_t(std::numeric_limits<std::int32_t>::max()):std::abs(std::int64_t(action.distance)-absolute),
            cylinder<0||action.cylinder<0?0:std::abs(std::int64_t(action.cylinder)-cylinder)};
        if(!best||rank<*best){best=rank;selected=&sample;}
    }
    return selected;
}
const RecordedSample* selectRecordedSample(const RecordedProfile& profile,std::string_view operation,std::int32_t distance,std::int32_t direction,std::int32_t cylinder){
    if(const auto* sample=selectRecordedSampleInProfile(profile,operation,distance,direction,cylinder))return sample;
    const auto fallback=baseRecordedProfile(profile.device);
    if(fallback&&*fallback!=profile.id)if(const auto* base=findRecordedProfile(*fallback))return selectRecordedSampleInProfile(*base,operation,distance,direction,cylinder);
    return nullptr;
}
}
