#include "Reader.h"
#include "RuntimeConstants.h"
#include "../../Formats/FormatIds.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::IsoReconstruction {
namespace {
std::shared_ptr<const MediaFileSystems::SectorImage> reconstruct(const Policy& policy,const std::optional<std::string>& format,const CandidateSet& candidates,std::stop_token token){
    checkCancellation(token);
    if(candidates.addressed.empty()&&candidates.physical.empty())throw MediaError(DiagnosticCode::IsoNoCandidates,{format.value_or(std::string(RuntimeConstants::AutomaticSelection)),std::to_string(candidates.addressed.entries().size()),std::to_string(candidates.physical.entries().size())});
    return build(policy,format,candidates,token);
}
}
Reader::Reader(DecodeFlux decode,ReadCapture read):candidates_(std::move(decode),std::move(read)){}
std::shared_ptr<const MediaFileSystems::SectorImage> Reader::read(const std::filesystem::path& path,std::optional<std::string> format,const CollectionProgressCallback& progress,std::stop_token token){
    checkCancellation(token);const auto policy=resolvePolicy(format);return reconstruct(policy,format,*candidates_.decode(path,policy.decoderIds,progress,token),token);
}
std::shared_ptr<const MediaFileSystems::SectorImage> Reader::read(const Flux::ProtectedTrackImage& image,std::optional<std::string> format,const CollectionProgressCallback& progress,std::stop_token token){
    checkCancellation(token);const auto policy=resolvePolicy(format);return reconstruct(policy,format,*candidates_.decode(image,policy.decoderIds,progress,token),token);
}
std::shared_ptr<const MediaFileSystems::SectorImage> Reader::readAtari(const std::filesystem::path& path,std::optional<std::string> format,const CollectionProgressCallback& progress,std::stop_token token){
    if(format&&!Primitives::asciiIdentifierStartsWithIgnoreCase(*format,Formats::AtariPrefix)&&!Primitives::asciiIdentifierStartsWithIgnoreCase(*format,Formats::AtariStPrefix))throw MediaError(DiagnosticCode::IsoRequestedFormat,{std::string(RuntimeConstants::AtariFamilies),*format});
    return read(path,std::move(format),progress,token);
}
}
