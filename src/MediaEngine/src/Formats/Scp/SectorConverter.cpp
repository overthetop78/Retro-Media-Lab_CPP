#include "SectorConverter.h"
#include "DiskTypes.h"
#include "../../Encoding/TimingConstants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Scp {
SectorConverter::SectorConverter()=default;
SectorConverter::SectorConverter(Encoding::SectorImageEncoder encoder):encoder_(std::move(encoder)){}
bool SectorConverter::canCreate(const MediaFileSystems::SectorImage& image)const{return encoder_.canEncode(image);}
Image SectorConverter::create(const MediaFileSystems::SectorImage& image,std::stop_token token)const {
    const auto encoded=encoder_.encodePhysical(image,token);
    if(encoded.empty())throw MediaError(DiagnosticCode::ImageEncodingNoTrack,{image.geometry().formatId});
    Image output;bool side0=false,side1=false;auto flags=Constants::ConversionFlags;
    for(const auto& item:encoded){
        checkCancellation(token);
        Track track;track.trackNumber=trackNumber(item.cylinder,item.head);track.cylinder=item.cylinder;track.head=item.head;
        track.revolutions.push_back(syntheticRevolution(item.track,Constants::CaptureResolution,token));
        output.tracks.push_back(std::move(track));side0|=item.head==0;side1|=item.head==1;
        if(item.cylinder>Constants::LowDensityLastCylinder)flags|=Constants::Tpi96Flag;
        if(item.track.revolution.indexTimeTicks==Encoding::TimingConstants::Rpm360Index)flags|=Constants::Rpm360Flag;
    }
    const auto bounds=std::minmax_element(output.tracks.begin(),output.tracks.end(),[](const auto&a,const auto&b){return a.trackNumber<b.trackNumber;});
    output.header={Constants::CaptureVersion,diskType(image),Constants::CaptureRevolutions,bounds.first->trackNumber,
        bounds.second->trackNumber,flags,Constants::DefaultWidth,
        side0&&side1?Constants::BothHeads:side0?Constants::Side0:Constants::Side1,Constants::CaptureResolution,Constants::MissingChecksum};
    output.checksumValid=true;output.fileSize=Constants::InitialFileSize;
    return output;
}
void SectorConverter::convert(const MediaFileSystems::SectorImage& image,const ImageOutput& output,std::stop_token token)const{write(create(image,token),output,token);}
void SectorConverter::convert(const MediaFileSystems::SectorImage& image,const std::filesystem::path& path,std::stop_token token)const{write(create(image,token),path,token);}
}

