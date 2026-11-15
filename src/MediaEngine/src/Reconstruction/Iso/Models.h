#pragma once
#include "../../Decoding/Models.h"
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include <map>
#include <tuple>
namespace RetroMediaLab::MediaEngine::IsoReconstruction {
struct Candidate {Decoding::Sector sector;std::int32_t revolution=0;std::optional<std::int32_t> sourceTrack;};
struct Group {MediaFileSystems::SectorAddress address;std::vector<Candidate> candidates;};
class Candidates {
public:
    RETROMEDIALAB_MEDIAENGINE_API std::vector<Candidate>& ensure(const MediaFileSystems::SectorAddress&);
    RETROMEDIALAB_MEDIAENGINE_API void add(const MediaFileSystems::SectorAddress&,Candidate);
    RETROMEDIALAB_MEDIAENGINE_API const std::vector<Candidate>* find(const MediaFileSystems::SectorAddress&) const;
    const std::vector<Group>& entries() const noexcept{return groups_;}
    bool empty() const noexcept{return groups_.empty();}
private:
    std::map<std::tuple<std::int64_t,std::int64_t,std::int64_t>,std::size_t> lookup_;
    std::vector<Group> groups_;
};
struct CandidateSet {Candidates addressed,physical;};
struct Measurement {std::int32_t sectorSize=0,cylinders=0,heads=0,sectorsPerTrack=0;std::vector<std::int64_t> sectorOrder;bool zeroBased=false;};
}
