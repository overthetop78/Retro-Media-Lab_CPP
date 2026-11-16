#include "Metadata.h"
#include "Constants.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::DiskMetadata {
std::optional<std::string> systemId(std::string_view format){for(const auto& rule:Constants::SystemRules)if(rule.exact?Primitives::ordinalIdentifierEqualsIgnoreCase(format,rule.format):Primitives::ordinalIdentifierStartsWithIgnoreCase(format,rule.format))return std::string(rule.id);return {};}
std::optional<std::string> protectionId(std::span<const std::string> formats){for(const auto& format:formats)if(Primitives::ordinalIdentifierEqualsIgnoreCase(format,Formats::AppleIIRwts18))return std::string(Formats::AppleIIRwts18);return {};}
DiskImageMetadata create(const MediaFileSystems::SectorImage& image,std::span<const std::string> detected,std::stop_token token){
 checkCancellation(token);std::vector<std::string> formats{image.geometry().formatId};
 const auto add=[](std::vector<std::string>& values,std::string_view value){if(std::none_of(values.begin(),values.end(),[&](const auto& existing){return Primitives::ordinalIdentifierEqualsIgnoreCase(existing,value);}))values.emplace_back(value);};
 for(const auto& format:detected)add(formats,format);
 DiskImageMetadata result;for(const auto& format:formats){checkCancellation(token);if(auto id=systemId(format))add(result.systemIds,*id);}
 result.protectionId=protectionId(formats);result.content=content(image,token);return result;
}
}
