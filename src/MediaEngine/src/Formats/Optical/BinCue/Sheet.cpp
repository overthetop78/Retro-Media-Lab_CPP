#include "Sheet.h"
#include "../../../Primitives/Identifiers.h"
#include "../../../Models/Optical/Internal.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <charconv>
#include <algorithm>
#include <unordered_set>
namespace RetroMediaLab::MediaEngine::BinCue {
namespace {
using namespace Constants;
QString text(std::string_view value){return Primitives::utf8Identifier(value);}std::string utf8(const QString& value){return value.toUtf8().toStdString();}
std::uint64_t number(const QString& value,std::uint64_t maximum,bool positive=false){const auto data=value.toUtf8();std::uint64_t result=0;const auto parsed=std::from_chars(data.data(),data.data()+data.size(),result);if(data.isEmpty()||parsed.ec!=std::errc()||parsed.ptr!=data.data()+data.size()||result>maximum||(positive&&!result))throw MediaError(DiagnosticCode::CueStructure);return result;}
qsizetype separator(const QString& value){for(qsizetype i=0;i<value.size();++i)if(value[i]==QChar(Space)||value[i]==QChar(Tab))return i;return -1;}
QStringList tokens(const QString& value){QStringList result;QString current;bool quoted=false;for(qsizetype i=0;i<value.size();++i){const auto c=value[i];if(c==QChar(Quote)){if(quoted&&i+1<value.size()&&value[i+1]==QChar(Quote)){current+=QChar(Quote);++i;}else quoted=!quoted;continue;}if(!quoted&&c.isSpace()){if(!current.isEmpty()){result.push_back(current);current.clear();}continue;}current+=c;}if(quoted)throw MediaError(DiagnosticCode::CueStructure);if(!current.isEmpty())result.push_back(current);return result;}
QString parseText(QString value){value=value.trimmed();if(value.size()>=TokenPair&&value.front()==QChar(Quote)&&value.back()==QChar(Quote)){value=value.mid(1,value.size()-TokenPair);return value.replace(QString(TokenPair,QChar(Quote)),QString(1,QChar(Quote)));}if(value.contains(QChar(Quote)))throw MediaError(DiagnosticCode::CueStructure);return value;}
std::string required(const QString& value){if(value.trimmed().isEmpty())throw MediaError(DiagnosticCode::CueStructure);return utf8(value);}
std::int64_t time(const QString& value){const auto parts=value.split(QChar(Colon));if(parts.size()!=TimeFields)throw MediaError(DiagnosticCode::CueStructure);const auto minutes=number(parts[0],MaximumSigned),seconds=number(parts[1],SecondsPerMinute-1),frames=number(parts[TokenPair],FramesPerSecond-1);return OpticalInternal::add(OpticalInternal::multiply(OpticalInternal::add(OpticalInternal::multiply(std::int64_t(minutes),SecondsPerMinute),std::int64_t(seconds)),FramesPerSecond),std::int64_t(frames));}
void addMetadata(OpticalMetadata& metadata,const std::string& key,const std::string& value){if(!metadata.emplace(key,value).second)throw MediaError(DiagnosticCode::CueStructure);}
struct FileBuilder {std::string path;FileKind kind;std::vector<TrackFields> tracks;OpticalMetadata pending;};
}
TrackDeclaration::TrackDeclaration(TrackFields value):fields_(std::move(value)){
    if(fields_.number<=0||fields_.indexes.empty()||fields_.pregap<0||fields_.postgap<0)throw MediaError(DiagnosticCode::CueStructure);std::unordered_set<int> indexes;for(const auto& index:fields_.indexes)if(!indexes.insert(index.number()).second)throw MediaError(DiagnosticCode::CueStructure);std::sort(fields_.indexes.begin(),fields_.indexes.end(),[](const auto& a,const auto& b){return a.number()<b.number();});
}
FileDeclaration::FileDeclaration(std::string path,FileKind kind,std::vector<TrackDeclaration> tracks):path_(std::move(path)),kind_(kind),tracks_(std::move(tracks)){if(Primitives::isBlankIdentifier(path_)||tracks_.empty())throw MediaError(DiagnosticCode::CueStructure);}
Sheet::Sheet(std::vector<FileDeclaration> files,std::optional<std::string> catalog,OpticalMetadata metadata):files_(std::move(files)),catalog_(std::move(catalog)),metadata_(std::move(metadata)){if(files_.empty())throw MediaError(DiagnosticCode::CueStructure);std::unordered_set<int> numbers;for(const auto& file:files_)for(const auto& track:file.tracks())if(!numbers.insert(track.fields().number).second)throw MediaError(DiagnosticCode::CueStructure);}
Sheet parseSheet(std::span<const std::string> lines,std::stop_token token){
    using namespace Constants;std::vector<FileBuilder> files;OpticalMetadata metadata;std::optional<std::string> catalog;bool activeTrack=false;std::size_t lineNumber=0;
    for(const auto& original:lines){++lineNumber;checkCancellation(token);const auto line=text(original).trimmed();if(line.isEmpty())continue;const auto split=separator(line);const auto command=(split<0?line:line.first(split)).toUpper(),argument=split<0?QString{}:line.mid(split+1).trimmed();
        try{
            const auto track=[&]()->TrackFields&{if(!activeTrack||files.empty()||files.back().tracks.empty())throw MediaError(DiagnosticCode::CueStructure);return files.back().tracks.back();};
            if(command==text(File)){const auto words=tokens(argument);if(words.size()<TokenPair)throw MediaError(DiagnosticCode::CueStructure);const auto type=words.back().toUpper();FileKind kind;if(type==text(BinaryFile))kind=FileKind::Binary;else if(type==text(WaveFile))kind=FileKind::WavePcm;else if(type==text(VorbisFile))kind=FileKind::Vorbis;else throw MediaError(DiagnosticCode::CueStructure);const auto offset=argument.lastIndexOf(words.back(),-1,Qt::CaseInsensitive);const auto path=parseText(argument.first(offset).trimmed());if(QFileInfo(path).suffix().compare(text(OggExtension.substr(1)),Qt::CaseInsensitive)==0)kind=FileKind::Vorbis;files.push_back({required(path),kind,{},{}});activeTrack=false;}
            else if(command==text(Track)){if(files.empty())throw MediaError(DiagnosticCode::CueStructure);const auto words=tokens(argument);if(words.size()!=TokenPair)throw MediaError(DiagnosticCode::CueStructure);TrackFields value;value.number=int(number(words[0],MaximumInt,true));const auto mode=words[1].toUpper();bool found=false;for(const auto& candidate:Modes)if(mode==text(candidate.name)){value.mode=candidate.mode;found=true;break;}if(!found)throw MediaError(DiagnosticCode::CueStructure);value.metadata=std::move(files.back().pending);files.back().pending.clear();files.back().tracks.push_back(std::move(value));activeTrack=true;}
            else if(command==text(Index)){auto& target=track();const auto words=tokens(argument);if(words.size()!=TokenPair)throw MediaError(DiagnosticCode::CueStructure);const auto index=int(number(words[0],MaximumInt));target.indexes.emplace_back(index,time(words[1]));}
            else if(command==text(Pregap))track().pregap=time(argument);
            else if(command==text(Postgap))track().postgap=time(argument);
            else if(command==text(Flags)){auto& target=track();for(const auto& word:tokens(argument))target.flags.push_back(utf8(word));}
            else if(command==text(Isrc))track().isrc=required(argument);
            else if(command==text(Catalog)){if(activeTrack)throw MediaError(DiagnosticCode::CueStructure);catalog=required(argument);}
            else if(command==text(Title)||command==text(Performer)||command==text(Songwriter))addMetadata(activeTrack?track().metadata:metadata,utf8(command),utf8(parseText(argument)));
            else if(command==text(Rem)){const auto position=separator(argument);const auto name=(position<0?argument:argument.first(position)).toUpper();const auto loop=name==text(Loop)||name==text(NoLoop);auto& target=activeTrack?track().metadata:!files.empty()&&loop?files.back().pending:metadata;
                if(loop){target.erase(std::string(RemLoop));target.erase(std::string(RemNoLoop));target[std::string(RemPrefix)+utf8(name)]=position<0?std::string{}:utf8(parseText(argument.mid(position+1).trimmed()));}
                else{if(position<=0)throw MediaError(DiagnosticCode::CueStructure);addMetadata(target,std::string(RemPrefix)+utf8(name),utf8(parseText(argument.mid(position+1).trimmed())));}}
            else throw MediaError(DiagnosticCode::CueStructure);
        }catch(const MediaError&){throw MediaError(DiagnosticCode::CueSyntax,{std::to_string(lineNumber),original});}
    }
    std::vector<FileDeclaration> result;for(auto& file:files){std::vector<TrackDeclaration> tracks;for(auto& value:file.tracks)tracks.emplace_back(std::move(value));result.emplace_back(std::move(file.path),file.kind,std::move(tracks));}return Sheet(std::move(result),std::move(catalog),std::move(metadata));
}
Sheet readSheet(const std::filesystem::path& path,std::stop_token token){const auto raw=path.u8string();QFile file(text(std::string_view(reinterpret_cast<const char*>(raw.data()),raw.size())));checkCancellation(token);if(!file.open(QIODevice::ReadOnly))throw MediaError(DiagnosticCode::CueIo,{file.fileName().toUtf8().toStdString()});QTextStream input(&file);std::vector<std::string> lines;while(!input.atEnd()){checkCancellation(token);lines.push_back(input.readLine().toUtf8().toStdString());}if(file.error()!=QFileDevice::NoError)throw MediaError(DiagnosticCode::CueIo,{file.fileName().toUtf8().toStdString()});return parseSheet(lines,token);}
}
