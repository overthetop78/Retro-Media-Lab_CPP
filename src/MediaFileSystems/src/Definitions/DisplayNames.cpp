#include <RetroMediaLab/MediaFileSystems/FileSystemDisplayNames.h>
#include <RetroMediaLab/MediaFileSystems/FormatIds.h>
#include "DisplayConstants.h"
#include "FileSystemNamesData.h"
#include "../Primitives/Identifiers.h"
#include <QString>
namespace RetroMediaLab::MediaFileSystems {
namespace C=DisplayConstants;
namespace {
bool same(std::string_view a,std::string_view b){return Identifiers::utf8(a).compare(Identifiers::utf8(b),Qt::CaseInsensitive)==0;}
std::string text(std::string_view key){for(const auto& entry:NameData::Entries)if(entry.key==key)return std::string(entry.text);return std::string(key);}
void replace(std::string& value,std::string_view token,std::string_view replacement){std::size_t position=0;while((position=value.find(token,position))!=std::string::npos){value.replace(position,token.size(),replacement);position+=replacement.size();}}
}
void applyFileSystemDisplayName(FileSystemVolume& volume,const SectorGeometry& geometry,std::optional<std::uint16_t> lisaVersion){
 std::string_view key;std::vector<std::string> arguments;const auto id=std::string_view(volume.fileSystemId);
 if(same(id,FileSystemIds::AppleDos))key=geometry.sectorsPerTrack==C::AppleDos32Sectors?C::AppleDos32:C::AppleDos33;
 else if(same(id,FileSystemIds::Fat12)){const auto format=Identifiers::utf8(geometry.formatId);key=format.startsWith(Identifiers::utf8(C::IbmPrefix),Qt::CaseInsensitive)?C::Fat12Ibm:format.startsWith(Identifiers::utf8(C::MsxPrefix),Qt::CaseInsensitive)?C::Fat12Msx:C::Fat12Atari;}
 else if(same(id,FileSystemIds::AmstradCpm))key=same(geometry.formatId,Formats::AmstradPcw)?C::AmstradPcw:C::AmstradCpc;
 else if(same(id,FileSystemIds::AtariClkGraphicsLibrary)){key=C::AtariClk;arguments={text(C::AtariClkProduct)};}
 else if(same(id,FileSystemIds::AtariBootDisk)){key=C::AtariBoot;arguments={text(C::AtariProduct)};}
 else if(same(id,FileSystemIds::Lisa)&&lisaVersion){arguments={text(C::LisaProduct)};switch(*lisaVersion){case C::LisaTableVersion:key=C::LisaTable;break;case C::LisaHashVersion:key=C::LisaHash;break;case C::LisaTreeVersion:key=C::LisaTree;arguments.push_back(text(C::LisaTreeType));break;default:key=C::LisaUnknown;arguments.push_back(QString::number(*lisaVersion,C::HexBase).rightJustified(C::VersionWidth,QLatin1Char(C::HexFill)).toUpper().toStdString());break;}}
 else for(const auto& [known,name]:C::Fixed)if(same(id,known)){key=name;break;}
 if(key.empty())return;
 volume.fileSystemDisplayName=text(key);if(!arguments.empty())replace(volume.fileSystemDisplayName,C::FirstParameter,arguments.front());if(arguments.size()>1)replace(volume.fileSystemDisplayName,C::SecondParameter,arguments[1]);
 volume.fileSystemDisplayResourceKey=key;volume.fileSystemDisplayTranslationDomain=C::Domain;volume.fileSystemDisplayArguments=std::move(arguments);
}
}
