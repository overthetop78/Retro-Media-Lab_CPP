#include <RetroMediaLab/MediaFileSystems/AmigaFlatArchive.h>
#include "Internal.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaFileSystems/Adfs.h>
#include <RetroMediaLab/MediaFileSystems/AmigaDos.h>
#include <RetroMediaLab/MediaFileSystems/Fat12.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Adf {
using namespace Constants;
bool canRead(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,std::string_view extension,std::optional<std::string_view> requested,std::stop_token token){
    checkCancellation(token);if(!source)throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::InvalidSource);
    const auto length=source->length();const auto supported=!requested||findGeometry(*requested).has_value();
    if(supported&&Internal::same(extension,AdfExtension)&&length<HighCapacity&&!Internal::acceptedSize(length)){
        const auto header=Internal::header(*source,token);
        if(auto candidate=Internal::partial(source,header,token);candidate&&(!requested||Internal::same(candidate->geometry().formatId,*requested)))return true;
    }
    if(Internal::same(extension,Adfs640Extension))return length==BbcLargeCapacity&&(!requested||Internal::same(*requested,AcornAdfs640));
    if(requested&&Internal::same(*requested,AcornDosPlus800))return length==Capacity;
    for(const auto& p:acornGeometries())if(p.geometry.blockSize==BbcBlockSize&&Internal::capacity(p)==length)
        return supported&&(requested?Internal::same(*requested,p.geometry.formatId):std::any_of(Extensions.begin(),Extensions.end(),[&](auto e){return Internal::same(e,extension);}));
    return supported&&Internal::acceptedSize(length);
}
std::shared_ptr<const MediaFileSystems::SectorImage> read(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,std::optional<std::string_view> requested,std::stop_token token){
    checkCancellation(token);if(!source)throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::InvalidSource);
    const auto length=source->length();for(const auto& p:amigaGeometries())if(Internal::capacity(p)==length)return Internal::image(std::move(source),p);
    const auto header=Internal::header(*source,token);
    if(auto candidate=Internal::partial(source,header,token);candidate&&(!requested||Internal::same(candidate->geometry().formatId,*requested)))return candidate;
    for(const auto& p:acornGeometries())if(p.geometry.blockSize==BbcBlockSize&&Internal::capacity(p)==length)return Internal::image(std::move(source),p);
    if(length==Capacity){const auto dos=(requested&&Internal::same(*requested,AcornDosPlus800))||(!requested&&hasDosPlusHeader(header));return Internal::image(std::move(source),*findGeometry(dos?AcornDosPlus800:AcornAdfs800));}
    if(length==PaddedCapacity)return Internal::image(std::move(source),*findGeometry(AcornAdfs800));
    if(length==HighCapacity)return Internal::image(std::move(source),*findGeometry(AcornAdfs1600));
    throw MediaError(DiagnosticCode::AdfInvalidLength,{std::to_string(length),Internal::acceptedSizes()});
}
namespace {
MediaFileSystems::FileSystemVolume exploreImage(const MediaFileSystems::SectorImage& image,std::stop_token token){
    const auto& id=image.geometry().formatId;
    auto volume=(Internal::same(id,AmigaDos)||Internal::same(id,AmigaDosHighDensity))?(MediaFileSystems::AmigaDos::canRead(image,token)?MediaFileSystems::AmigaDos::read(image,token):MediaFileSystems::AmigaFlatArchive::canRead(image,token)?MediaFileSystems::AmigaFlatArchive::read(image,token):MediaFileSystems::AmigaDos::read(image,token)):Internal::same(id,AcornDosPlus800)?MediaFileSystems::Fat12::read(image,token):MediaFileSystems::Adfs::read(image,token);
    MediaFileSystems::analyzeEntries(id,volume,token);return volume;
}
}
MediaFileSystems::FileSystemVolume explore(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,std::optional<std::string_view> requested,std::stop_token token){const auto image=read(std::move(source),requested,token);return exploreImage(*image,token);}
MediaFileSystems::FileSystemVolume exploreAcorn(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,std::optional<std::string_view> requested,std::stop_token token){
    const auto image=read(std::move(source),requested,token);const auto& id=image->geometry().formatId;
    if(Internal::same(id,AmigaDos)||Internal::same(id,AmigaDosHighDensity))throw MediaError(DiagnosticCode::AdfNoAcornCatalogue,{id});
    return exploreImage(*image,token);
}
}
