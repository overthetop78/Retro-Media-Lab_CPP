#include "Catalogue.h"
#include "../../Primitives/Identifiers.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
std::vector<DiskFormat> mergeRuntimeImageFormats(std::span<const DiskFormat> curated,std::span<const DiskFormat> runtime){
    std::vector<DiskFormat> result(curated.begin(),curated.end());
    for(const auto& format:runtime){
        const auto found=std::find_if(result.begin(),result.end(),[&](const auto& item){return Primitives::ordinalIdentifierEqualsIgnoreCase(item.id,format.id);});
        if(found==result.end())result.push_back(format);
        else{found->extensions=format.extensions;found->compatibleSourceExtensions=format.compatibleSourceExtensions;}
    }
    return result;
}
std::string imageFormatName(const DiskFormat& format,const FormatTextResolver& resolve){return format.displayNameKey.empty()?format.technicalDisplayName.value_or(format.id):resolve?resolve(format.displayNameKey):format.displayNameKey;}
std::string imageFormatFamilyName(const DiskFormat& format,const FormatTextResolver& resolve){return format.familyNameKey.empty()?format.family:resolve?resolve(format.familyNameKey):format.familyNameKey;}
std::string imageExtensionName(const ImageExtension& extension,const FormatTextResolver& resolve){return extension.displayNameKey.empty()?extension.technicalDisplayName.value_or(extension.extension):resolve?resolve(extension.displayNameKey):extension.displayNameKey;}
}
