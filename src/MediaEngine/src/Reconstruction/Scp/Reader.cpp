#include "Reader.h"
#include "../../Images/ReadRejection.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::ScpReconstruction {
Reader::Reader(Candidates candidates,MediaFileSystems::SectorRegistry fileSystems):candidates_(std::move(candidates)),fileSystems_(std::move(fileSystems)){}
Image Reader::read(const std::filesystem::path& path,std::optional<std::string> format,const Progress& progress,std::stop_token token)const {
    MediaFileSystems::checkCancellation(token);
    if(const auto candidate=candidates_.selected(format))return candidate->readPath(path,format,progress,token);
    return readDefaults([&](const Candidate& candidate){return candidate.readPath(path,{},progress,token);},false,token);
}
Image Reader::read(const Flux::ProtectedTrackImage& flux,std::optional<std::string> format,const Progress& progress,std::stop_token token)const {
    MediaFileSystems::checkCancellation(token);
    if(const auto candidate=candidates_.selected(format)){
        if(!candidate->readFlux)throw MediaError(DiagnosticCode::UnsupportedRawFormat,{candidate->id});
        return candidate->readFlux(flux,format,progress,token);
    }
    return readDefaults([&](const Candidate& candidate){return candidate.readFlux(flux,{},progress,token);},true,token);
}
Image Reader::readDefaults(const std::function<Image(const Candidate&)>& read,bool fluxOnly,std::stop_token token)const {
    Image first;
    std::vector<DiagnosticCause> failures;
    for(const auto& candidate:candidates_.defaults()){
        MediaFileSystems::checkCancellation(token);
        if(fluxOnly&&!candidate.readFlux)continue;
        try{
            auto image=read(candidate);
            if(!image)throw MediaError(DiagnosticCode::ReaderNullDocument);
            if(!first)first=image;
            if(fileSystems_.tryRead(*image,{},token))return image;
        }catch(...){
            auto failure=std::current_exception();
            if(!isImageReadRejection(failure))std::rethrow_exception(failure);
            failures.push_back({candidate.id,std::move(failure)});
        }
    }
    if(first)return first;
    throw MediaError(DiagnosticCode::ReaderAllRejected,{},std::move(failures));
}
}
