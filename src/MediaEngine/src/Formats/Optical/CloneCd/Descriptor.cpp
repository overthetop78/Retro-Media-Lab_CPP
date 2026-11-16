#include "Descriptor.h"
#include "../../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QFile>
#include <QTextStream>
#include <charconv>
#include <bit>
#include <algorithm>
#include <limits>
#include <unordered_set>
namespace RetroMediaLab::MediaEngine::CloneCd {
namespace {
using namespace Constants;
using Properties=std::map<std::string,std::string,OpticalMetadataLess>;
using Sections=std::map<std::string,Properties,OpticalMetadataLess>;
QString text(std::string_view value){return Primitives::utf8Identifier(value);}
int number(std::string_view value){
    auto string=text(value).trimmed().toUtf8().toStdString();bool negative=false;
    if(!string.empty()&&string.front()==Minus){negative=true;string.erase(0,1);}
    const auto hex=Primitives::asciiIdentifierStartsWithIgnoreCase(string,HexPrefix);if(hex)string.erase(0,HexPrefix.size());
    std::uint32_t raw=0;const auto parsed=std::from_chars(string.data(),string.data()+string.size(),raw,hex?HexBase:DecimalBase);
    if(string.empty()||parsed.ec!=std::errc()||parsed.ptr!=string.data()+string.size()||(!hex&&raw>std::uint32_t(std::numeric_limits<std::int32_t>::max())))throw MediaError(DiagnosticCode::CloneCdDescriptor);
    const auto signedValue=std::bit_cast<std::int32_t>(raw);if(negative&&signedValue==std::numeric_limits<std::int32_t>::min())throw MediaError(DiagnosticCode::CloneCdDescriptor);return negative?-signedValue:signedValue;
}
int indexed(std::string_view name,std::string_view prefix){const auto string=text(name.substr(prefix.size())).trimmed().toUtf8().toStdString();int value=0;const auto result=std::from_chars(string.data(),string.data()+string.size(),value);if(string.empty()||string.front()==Minus||result.ec!=std::errc()||result.ptr!=string.data()+string.size()||value<0)throw MediaError(DiagnosticCode::CloneCdDescriptor);return value;}
int required(const Properties& values,std::string_view key){const auto found=values.find(std::string(key));if(found==values.end())throw MediaError(DiagnosticCode::CloneCdDescriptor);return number(found->second);}
const Properties& section(const Sections& values,std::string_view key){const auto found=values.find(std::string(key));if(found==values.end())throw MediaError(DiagnosticCode::CloneCdDescriptor);return found->second;}
Sections sections(std::span<const std::string> lines,std::stop_token token){Sections result;Properties* current=nullptr;for(const auto& source:lines){checkCancellation(token);const auto line=text(source).trimmed();if(line.isEmpty()||line.front()==QChar(Semicolon)||line.front()==QChar(Hash))continue;
    if(line.front()==QChar(Open)&&line.back()==QChar(Close)){const auto name=line.mid(1,line.size()-SectionBrackets).trimmed().toUtf8().toStdString();auto [position,inserted]=result.emplace(name,Properties{});if(name.empty()||!inserted)throw MediaError(DiagnosticCode::CloneCdDescriptor);current=&position->second;continue;}
    if(!current)throw MediaError(DiagnosticCode::CloneCdDescriptor);const auto separator=line.indexOf(QChar(Equals));if(separator<=0)throw MediaError(DiagnosticCode::CloneCdDescriptor);const auto key=line.first(separator).trimmed().toUtf8().toStdString();const auto value=line.mid(separator+1).trimmed().toUtf8().toStdString();if(!current->emplace(key,value).second)throw MediaError(DiagnosticCode::CloneCdDescriptor);
}return result;}
TocEntry entry(int index,const Properties& values){return {index,required(values,SessionKey),required(values,PointKey),required(values,AdrKey),required(values,ControlKey),required(values,TrackNumberKey),required(values,AbsoluteMinuteKey),required(values,AbsoluteSecondKey),required(values,AbsoluteFrameKey),required(values,AbsoluteLbaKey),required(values,ZeroKey),required(values,PointMinuteKey),required(values,PointSecondKey),required(values,PointFrameKey),required(values,AbsoluteSectorKey)};}
BinCue::TrackDeclaration track(int index,const Properties& values){BinCue::TrackFields fields;fields.number=index;switch(required(values,ModeKey)){case AudioMode:fields.mode=OpticalTrackMode::Audio;break;case Mode1Mode:fields.mode=OpticalTrackMode::Mode1Raw2352;break;case Mode2Mode:fields.mode=OpticalTrackMode::Mode2Raw2352;break;default:throw MediaError(DiagnosticCode::CloneCdProfile);}
    for(const auto& [key,value]:values)if(Primitives::ordinalIdentifierStartsWithIgnoreCase(key,IndexPrefix))fields.indexes.emplace_back(indexed(key,IndexPrefix),number(value));
    if(const auto flags=values.find(std::string(FlagsKey));flags!=values.end())for(const auto& value:text(flags->second).split(QChar(Space),Qt::SkipEmptyParts)){const auto flag=value.trimmed();if(!flag.isEmpty())fields.flags.push_back(flag.toUtf8().toStdString());}
    if(const auto isrc=values.find(std::string(IsrcKey));isrc!=values.end())fields.isrc=isrc->second;
    return BinCue::TrackDeclaration(std::move(fields));
}
}
Descriptor::Descriptor(int version,int sessionCount,std::optional<std::string> catalog,std::vector<TocEntry> entries,std::vector<BinCue::TrackDeclaration> tracks,Sessions sessions):version_(version),sessionCount_(sessionCount),catalog_(std::move(catalog)),entries_(std::move(entries)),tracks_(std::move(tracks)),sessions_(std::move(sessions)){
    if(version_<=0||sessionCount_<=0||entries_.empty()||tracks_.empty()||sessions_.size()!=std::size_t(sessionCount_))throw MediaError(DiagnosticCode::CloneCdDescriptor);
}
Descriptor parseDescriptor(std::span<const std::string> lines,std::stop_token token){using namespace Constants;const auto values=sections(lines,token);const auto version=required(section(values,HeaderSection),VersionKey);if(version<MinimumVersion||version>MaximumVersion)throw MediaError(DiagnosticCode::CloneCdProfile);const auto& disc=section(values,DiscSection);const auto count=required(disc,SessionsKey),tocCount=required(disc,EntriesKey);if(count<=0||tocCount<=0)throw MediaError(DiagnosticCode::CloneCdDescriptor);
    Sessions sessions;std::vector<TocEntry> entries;std::vector<BinCue::TrackDeclaration> tracks;
    for(const auto& [name,properties]:values){checkCancellation(token);if(Primitives::ordinalIdentifierStartsWithIgnoreCase(name,SessionPrefix)){SessionProperties fields;for(const auto& [key,value]:properties)fields.emplace(key,number(value));if(!sessions.emplace(indexed(name,SessionPrefix),std::move(fields)).second)throw MediaError(DiagnosticCode::CloneCdDescriptor);}
        else if(Primitives::ordinalIdentifierStartsWithIgnoreCase(name,EntryPrefix))entries.push_back(entry(indexed(name,EntryPrefix),properties));
        else if(Primitives::ordinalIdentifierStartsWithIgnoreCase(name,TrackPrefix))tracks.push_back(track(indexed(name,TrackPrefix),properties));}
    if(sessions.size()!=std::size_t(count)||entries.size()!=std::size_t(tocCount))throw MediaError(DiagnosticCode::CloneCdDescriptor);std::stable_sort(entries.begin(),entries.end(),[](const auto& a,const auto& b){return a.entryNumber<b.entryNumber;});std::stable_sort(tracks.begin(),tracks.end(),[](const auto& a,const auto& b){return a.fields().number<b.fields().number;});
    std::unordered_set<int> numbers;for(const auto& declaration:tracks)if(!numbers.insert(declaration.fields().number).second)throw MediaError(DiagnosticCode::CloneCdDescriptor);
    std::optional<std::string> catalog;if(const auto found=disc.find(std::string(CatalogKey));found!=disc.end())catalog=found->second;return Descriptor(version,count,std::move(catalog),std::move(entries),std::move(tracks),std::move(sessions));
}
Descriptor readDescriptor(const std::filesystem::path& path,std::stop_token token){const auto value=path.u8string();const std::string name(value.begin(),value.end());QFile file(text(name));checkCancellation(token);if(!file.open(QIODevice::ReadOnly))throw MediaError(DiagnosticCode::CloneCdIo,{name});QTextStream input(&file);std::vector<std::string> lines;while(!input.atEnd()){checkCancellation(token);lines.push_back(input.readLine().toUtf8().toStdString());}if(file.error()!=QFileDevice::NoError)throw MediaError(DiagnosticCode::CloneCdIo,{name});return parseDescriptor(lines,token);}
}
