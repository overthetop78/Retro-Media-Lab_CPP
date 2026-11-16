#include <RetroMediaLab/MediaFileSystems/FileSystemDisplayNames.h>
#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::Cpm {
bool canRead(const SectorImage& image,std::stop_token token){
    checkCancellation(token);const auto configured=Internal::genericLayout(image.geometry().formatId);if(!configured||std::find(Constants::SupportedSectorSizes.begin(),Constants::SupportedSectorSizes.end(),image.geometry().blockSize)==Constants::SupportedSectorSizes.end())return false;
    const auto logical=Internal::flatten(image,token);const auto layout=Internal::epsonLayout(image.geometry().formatId,logical,*configured,token);return Internal::score(logical,layout,true,token)>=Constants::MinimumDirectoryScore;
}
FileSystemVolume read(const SectorImage& image,std::stop_token token){
    checkCancellation(token);const auto configured=Internal::genericLayout(image.geometry().formatId);if(!configured)throw MediaError(DiagnosticCode::CpmMissingLayout,{image.geometry().formatId});
    const auto logical=Internal::flatten(image,token);const auto layout=Internal::epsonLayout(image.geometry().formatId,logical,*configured,token);if(Internal::score(logical,layout,true,token)<Constants::MinimumDirectoryScore)throw MediaError(DiagnosticCode::CpmUnsupportedDirectory,{image.geometry().formatId});const auto directory=Internal::directory(logical,layout,true,token);auto result=Internal::volume(image,logical,layout,directory,Constants::Id,token);applyFileSystemDisplayName(result,image.geometry());return result;
}
bool canReadAmstrad(const SectorImage& image,std::stop_token token){
    checkCancellation(token);if(std::none_of(Constants::AmstradFormats.begin(),Constants::AmstradFormats.end(),[&](auto format){return Internal::equal(image.geometry().formatId,format);}))return false;
    const auto logical=Internal::flatten(image,token);const auto layout=Internal::amstradLayout(image,logical,token);return layout&&Internal::looksLike(logical,*layout,Internal::equal(image.geometry().formatId,Formats::AmstradPcw),false,token);
}
FileSystemVolume readAmstrad(const SectorImage& image,std::stop_token token){
    checkCancellation(token);const auto logical=Internal::flatten(image,token);const auto layout=Internal::amstradLayout(image,logical,token);if(!layout)throw MediaError(DiagnosticCode::CpmMissingLayout,{image.geometry().formatId});if(!Internal::looksLike(logical,*layout,Internal::equal(image.geometry().formatId,Formats::AmstradPcw),false,token))throw MediaError(DiagnosticCode::CpmUnsupportedDirectory,{image.geometry().formatId});const auto directory=Internal::directory(logical,*layout,false,token);auto result=Internal::volume(image,logical,*layout,directory,Constants::AmstradId,token);applyFileSystemDisplayName(result,image.geometry());return result;
}
}
