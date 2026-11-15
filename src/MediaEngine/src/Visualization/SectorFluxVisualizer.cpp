#include "SectorFluxVisualizer.h"
#include "../Formats/Scp/Writer.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaEngine::Visualization {
SectorFluxVisualizer::SectorFluxVisualizer()=default;
SectorFluxVisualizer::SectorFluxVisualizer(Encoding::SectorImageEncoder encoder):encoder_(std::move(encoder)){}
bool SectorFluxVisualizer::canVisualize(const MediaFileSystems::SectorImage& image)const{return encoder_.canEncode(image);}
Scp::Image SectorFluxVisualizer::create(const MediaFileSystems::SectorImage& image,std::stop_token token)const {
    auto encoded=encoder_.encode(image,token);
    Scp::Image output;
    for(auto& item:encoded){
        checkCancellation(token);
        Scp::Track track;track.trackNumber=Scp::trackNumber(item.cylinder,item.head);track.cylinder=item.cylinder;track.head=item.head;
        if(item.track.revolution.intervals.size()>std::numeric_limits<std::uint32_t>::max())throw MediaError(DiagnosticCode::ScpOverflow);
        const auto count=std::uint32_t(item.track.revolution.intervals.size());
        // Preserve the C# constructor's default origin and direct interval metadata.
        track.revolutions.push_back({std::move(item.track.revolution),count,Scp::RevolutionOrigin::Captured});
        output.tracks.push_back(std::move(track));
    }
    if(output.tracks.empty())throw MediaError(DiagnosticCode::ImageEncodingNoTrack,{image.geometry().formatId});
    const auto bounds=std::minmax_element(output.tracks.begin(),output.tracks.end(),[](const auto&a,const auto&b){return a.trackNumber<b.trackNumber;});
    bool side0=false,side1=false;
    for(const auto& track:output.tracks){side0|=track.head==0;side1|=track.head==1;}
    output.header={Scp::Constants::VisualizationVersion,Scp::Constants::VisualizationDiskType,Scp::Constants::CaptureRevolutions,
        bounds.first->trackNumber,bounds.second->trackNumber,Scp::Constants::VisualizationFlags,Scp::Constants::DefaultWidth,
        side0&&side1?Scp::Constants::BothHeads:side0?Scp::Constants::Side0:Scp::Constants::Side1,
        Scp::Constants::CaptureResolution,Scp::Constants::MissingChecksum};
    output.checksumValid=true;output.fileSize=image.capacity();
    return output;
}
}

