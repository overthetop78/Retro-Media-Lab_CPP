#include "Writing.h"
#include "Constants.h"
#include "Raw.h"
#include "../FormatIds.h"
#include "../Nib/Nib.h"
#include "../Nib/Constants.h"
#include "../Woz/Woz.h"
#include "../Woz/Constants.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Apple {
namespace {bool same(std::string_view a,std::string_view b){return Primitives::ordinalIdentifierEqualsIgnoreCase(a,b);}}
bool sectorTarget(std::string_view format,std::string_view extension){
    using namespace Constants;
    if(same(extension,TwoMg))return same(format,Formats::AppleIIAppleDos113)||same(format,Formats::AppleIIAppleDos140)||isProDos140(format)||same(format,Formats::AppleIIProDos800);
    if(same(format,Formats::AppleIIAppleDos113))return same(extension,D13);
    if(same(format,Formats::AppleIIAppleDos140))return same(extension,Do)||same(extension,Dsk);
    if(isProDos140(format))return same(extension,Po)||same(extension,Do)||same(extension,Dsk);
    return same(format,Formats::AppleIIProDos800)&&same(extension,Po);
}
bool canWriteSector(const MediaFileSystems::SectorImage& image,std::string_view target){
    if(Primitives::isBlankIdentifier(target))throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::WriterTargetFormat);
    const auto& source=image.geometry().formatId;
    return (same(target,Formats::AppleIIAppleDos113)&&same(source,Formats::AppleIIDos32))
        ||(same(target,Formats::AppleIIAppleDos140)&&(same(source,Formats::AppleIIDos33)||same(source,Formats::AppleIIAppleDos140)))
        ||(same(target,Formats::AppleIIProDos140)&&(same(source,Formats::AppleIIProDos)||same(source,Formats::AppleIIProDos140)))
        ||(same(target,Formats::AppleIIProDos800)&&(same(source,Formats::AppleIIProDos)||same(source,Formats::AppleIIProDos800)))
        ||(same(target,Formats::AppleIIISos)&&same(source,Formats::AppleIIISos));
}
void validateSector(const MediaFileSystems::SectorImage& image,std::string_view target){if(!canWriteSector(image,target))throw MediaError(DiagnosticCode::AppleFileSystemChange,{image.geometry().formatId,std::string(target)});}
bool DiskImageWriter::supportsExtension(std::string_view extension){return same(extension,Constants::Nib)||same(extension,Constants::Woz);}
bool nibbleTarget(std::string_view format,std::string_view extension){return DiskImageWriter::supportsExtension(extension)&&(same(format,Formats::AppleIIRwts18)||TrackEncodingService::supportsStandard(format));}
void DiskImageWriter::write(const MediaFileSystems::SectorImage& image,const std::filesystem::path& output,std::stop_token token)const{
    const auto u8=output.extension().u8string();const std::string extension(u8.begin(),u8.end());
    if(!supportsExtension(extension))throw MediaError(DiagnosticCode::AppleOutputExtension,{extension});
    const bool nib=same(extension,Constants::Nib);const auto maximum=std::int32_t(nib?Nib::Constants::MaximumTrackBitCount:Woz::Constants::Woz1MaximumBitCount);
    const auto tracks=same(image.geometry().formatId,Formats::AppleIIRwts18)?encoder_.encodeRwts18(image,maximum,token):encoder_.encodeStandard(image,maximum,token);
    if(writer_){const auto bytes=nib?Nib::build(tracks):Woz::build(tracks);MediaFileSystems::checkCancellation(token);writer_(output,bytes,token);}
    else if(nib)Nib::write(tracks,output,token);else Woz::write(tracks,output,token);
}
}
