#pragma once
#include "../../Reconstruction/Scp/Candidates.h"
#include "../../Formats/Scp/CaptureReader.h"
#include "../../Decoding/Decoder.h"
#include "../../Images/Recognition/Interpretations.h"
#include <RetroMediaLab/MediaFileSystems/SectorRegistry.h>
namespace RetroMediaLab::MediaEngine::ScpInspection {
using Image=ScpReconstruction::Image;
using Match=MediaFileSystems::SectorFileSystemCandidate;
using MatchOwner=std::shared_ptr<const Match>;
enum class ProgressKind {FormatProbeStarted,FormatProbeCompleted,CandidateStarted,RevolutionDecoded,TrackDecoded,CandidateCompleted};
struct ProgressEvent {ProgressKind kind;std::string detail;std::size_t completed,total;std::optional<bool> recognized;std::optional<std::int32_t> cylinder,head;};
using Progress=std::function<void(const ProgressEvent&)>;
struct Inspection {std::string candidateId;Image image;std::vector<MatchOwner> matches;std::exception_ptr failure;};
struct Ranking {Image bestDecoded,bestRecognized;MatchOwner bestFileSystem;std::vector<MatchOwner> detected;std::vector<Image> decodedImages;std::vector<Inspection> rejected;};
struct Result {Scp::SharedImage capture;Ranking ranking;std::vector<MatchOwner> detected;std::vector<Image> credibleImages;};
RETROMEDIALAB_MEDIAENGINE_API std::vector<const Scp::Track*> sampleTracks(const Scp::Image&);
RETROMEDIALAB_MEDIAENGINE_API std::set<ScpReconstruction::Family> probeFamilies(const Scp::Image&,const Decoding::DecodeOperation&,const Progress& = {},std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API Image normalizeCompatibleGeometry(Image,const Scp::Image&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API double decodeScore(const MediaFileSystems::SectorImage&,std::stop_token={});
RETROMEDIALAB_MEDIAENGINE_API Ranking rank(std::vector<Inspection>,const Scp::Image&,std::stop_token={});
class Inspector final {
public:
 RETROMEDIALAB_MEDIAENGINE_API Inspector(MediaFileSystems::SectorRegistry,SectorInterpretations);
 RETROMEDIALAB_MEDIAENGINE_API std::vector<MatchOwner> inspectFileSystems(Image,std::stop_token={})const;
 RETROMEDIALAB_MEDIAENGINE_API Inspection inspect(const ScpReconstruction::Candidate&,const std::filesystem::path&,const ScpReconstruction::Progress& = {},std::stop_token={})const;
private:MediaFileSystems::SectorRegistry fileSystems_;SectorInterpretations interpretations_;
};
class Service final {
public:
 RETROMEDIALAB_MEDIAENGINE_API Service(std::shared_ptr<const Scp::CaptureReader>,ScpReconstruction::Candidates,Decoding::DecodeOperation,Inspector);
 RETROMEDIALAB_MEDIAENGINE_API Result explore(const std::filesystem::path&,const Progress& = {},std::stop_token={})const;
 RETROMEDIALAB_MEDIAENGINE_API Result explore(const std::filesystem::path&,Scp::SharedImage,const Progress& = {},std::stop_token={})const;
private:std::shared_ptr<const Scp::CaptureReader> captures_;ScpReconstruction::Candidates candidates_;Decoding::DecodeOperation decode_;Inspector inspector_;
};
}
