#pragma once
#include "../Iso/Collector.h"
#include <RetroMediaLab/MediaFileSystems/SectorImage.h>
#include <filesystem>
#include <map>
#include <set>
namespace RetroMediaLab::MediaEngine::ScpReconstruction {
enum class Family {Iso,Amiga,Commodore,Apple,Dec};
using Image=std::shared_ptr<const MediaFileSystems::SectorImage>;
using Progress=IsoReconstruction::CollectionProgressCallback;
using PathRead=std::function<Image(const std::filesystem::path&,const std::optional<std::string>&,const Progress&,std::stop_token)>;
using FluxRead=std::function<Image(const Flux::ProtectedTrackImage&,const std::optional<std::string>&,const Progress&,std::stop_token)>;
struct Candidate {
    std::string id;
    Family family;
    PathRead readPath;
    FluxRead readFlux;
};
struct Selection {std::function<bool(std::string_view)> matches;Candidate candidate;};
// Owns registrations and callback captures; returned references live as long as the registry.
class Candidates final {
public:
    RETROMEDIALAB_MEDIAENGINE_API Candidates(std::vector<Selection>,std::vector<Candidate>,std::map<Family,std::vector<Candidate>>,std::vector<Family>,Candidate fallback);
    RETROMEDIALAB_MEDIAENGINE_API const Candidate* selected(const std::optional<std::string>&)const;
    RETROMEDIALAB_MEDIAENGINE_API std::span<const Candidate> defaults()const noexcept;
    RETROMEDIALAB_MEDIAENGINE_API std::vector<Candidate> automatic(const std::set<Family>&)const;
private:
    std::vector<Selection> selections_;
    std::vector<Candidate> defaults_;
    std::map<Family,std::vector<Candidate>> families_;
    std::vector<Family> order_;
    Candidate fallback_;
};
}
