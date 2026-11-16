#include <RetroMediaLab/MediaEngine/DiskClassification.h>
#include "ClassificationConstants.h"
#include <RetroMediaLab/MediaEngine/BuiltInDiskDefinitions.h>
#include "../Formats/Definitions/Constants.h"
#include "../Formats/Scp/Constants.h"
#include "../Primitives/Identifiers.h"
#include "../Primitives/Path.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace {
namespace C=Visualization::ClassificationConstants;
bool same(std::string_view a,std::string_view b){return Primitives::ordinalIdentifierEqualsIgnoreCase(a,b);}
template<class Range>bool contains(const Range& values,std::string_view wanted){return std::any_of(std::begin(values),std::end(values),[&](const auto& value){return same(value,wanted);});}
std::optional<std::string> decoder(std::optional<std::string_view> machine,bool automatic){if(automatic&&(!machine||Primitives::isBlankIdentifier(*machine)))return {};if(machine)for(const auto& [name,id]:C::DecoderByMachine)if(same(name,*machine))return std::string(id);return std::string(C::IsoMfm);}
DiskMediaCategory category(std::optional<std::string_view> machine,std::optional<std::string_view> format){
 if(!machine)return DiskMediaCategory::Unknown;if(same(*machine,C::Amstrad))return DiskMediaCategory::ThreeInch;if(same(*machine,C::Dec))return DiskMediaCategory::EightInch;
 const auto id=Primitives::utf8Identifier(format.value_or(std::string_view{})).toLower();const auto has=[&](auto marker){return id.contains(Primitives::utf8Identifier(marker));};
 if(contains(C::ThreeHalfMachines,*machine))return has(C::HighDensity)||has(C::ExtendedDensity)||has(C::DensitySuffix)?DiskMediaCategory::ThreeHalfHd:DiskMediaCategory::ThreeHalfDd;
 if(contains(C::FiveQuarterMachines,*machine))return has(C::DensityMarker)?DiskMediaCategory::FiveQuarterHd:DiskMediaCategory::FiveQuarterDd;return DiskMediaCategory::Unknown;
}
}
DiskVisualizationClassification classifyDiskVisualization(std::optional<std::string_view> machine,std::optional<std::string_view> format,std::optional<std::string_view> protection,bool automatic){return {protection?std::optional<std::string>(std::string(*protection)):decoder(machine,automatic),category(machine,format)};}
bool canConvertToScpForVisualization(const std::filesystem::path& path,const DiskFormat* format,const GwFormatCapabilities& capabilities){
 if(!capabilities.isKnown())return false;const auto suffix=Primitives::pathExtension(path);const auto& supported=capabilities.imageExtensions;
 if(same(suffix,C::Dsk)||same(suffix,C::Edsk))return contains(supported,suffix)&&contains(supported,Scp::Constants::Extension);
 if(!format)return false;if(same(suffix,C::Atr))return (format->id==Formats::Atari90||format->id==Formats::Atari130)&&contains(capabilities.formatIds,format->id)&&contains(supported,Definitions::Constants::RawExtension)&&contains(supported,Scp::Constants::Extension);
 const auto argument=gwFormatArgument(format->id);return !same(suffix,Scp::Constants::Extension)&&argument&&(contains(capabilities.formatIds,*argument)||supportsBuiltInDiskDefinition(format->id))&&contains(supported,suffix)&&contains(supported,Scp::Constants::Extension);
}
}
