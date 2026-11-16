#pragma once
#include <string_view>
#include <array>
#include <utility>
namespace RetroMediaLab::MediaEngine::CatalogConstants {
inline constexpr std::string_view Resource=":/MediaEngine/FormatCatalog/formats.json";
inline constexpr std::string_view Id="id",Family="family",FamilyKey="familyNameKey",DisplayKey="displayNameKey",Extensions="extensions",Extension="extension",Default="isDefault",Common="isCommon",Sources="compatibleSourceExtensions",Tag="tag",FormFactor="formFactor",PhysicalRead="supportsPhysicalRead",PhysicalWrite="supportsPhysicalWrite",Density="density";
inline constexpr char ExtensionSeparator='.';
inline constexpr std::string_view FormatSection="FORMAT options:",SuffixSection="Supported file suffixes:";
inline constexpr std::string_view FormatPattern=R"((?<![\w.])[a-z][a-z0-9_-]*(?:\.[a-z0-9_-]+)+(?![\w.]))",ExtensionPattern=R"((?<![\w.])\.[a-z0-9][a-z0-9_-]*(?![\w.]))";
inline constexpr std::string_view TagPattern="[^A-Z0-9]+",WordSeparatorPattern="[-]+",DetailSeparator=" · ",NameSeparator=" — ",WordSeparator=" ",ImgExtension=".img";
inline constexpr char Hyphen='-',Underscore='_';
inline constexpr std::array<std::string_view,4> BuiltInDefinitions{"atarist.810","amstrad.cpc","amstrad.pcw","ucsd.ibm.mfm"};
inline constexpr std::array<std::string_view,3> WorkspaceExtensions{".img",".scp",".hfe"};
inline constexpr std::array<std::string_view,4> FallbackExtensions{".scp",".img",".ima",".hfe"};
inline constexpr std::pair<std::string_view,std::string_view> FriendlyTokens[]{
    {"ibm","IBM PC"},{"pc98","PC-98"},{"msx","MSX"},{"dec","DEC"},{"hp","HP"},{"rm","RM"},{"tsc","TSC"},{"zx","ZX Spectrum"},{"coco","TRS-80 CoCo"},{"apple2","Apple II"},{"atarist","Atari ST"},
    {"rx01","RX01"},{"rx02","RX02"},{"fm","FM"},{"mfm","MFM"},{"gcr","GCR"},{"adfs","ADFS"},{"dos","DOS"},{"hd","HD"},{"dd","DD"}
};
}
