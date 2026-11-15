#include "DiskDefinitions.h"
#include "Constants.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>
#include <QTextStream>
#include <algorithm>
#include <set>

namespace RetroMediaLab::MediaEngine::Definitions {
namespace {
QString qtext(std::string_view value){return QString::fromUtf8(value.data(),qsizetype(value.size()));}
std::string utf8(const QString& value){return value.toUtf8().toStdString();}
const QRegularExpression& diskPattern(){static const QRegularExpression pattern(qtext(Constants::DiskPattern),QRegularExpression::CaseInsensitiveOption);return pattern;}
const QRegularExpression& importPattern(){static const QRegularExpression pattern(qtext(Constants::ImportPattern),QRegularExpression::CaseInsensitiveOption);return pattern;}
const QRegularExpression& wordsPattern(){static const QRegularExpression pattern(qtext(Constants::WordsPattern));return pattern;}
struct ActiveScope {std::set<QString>& files;QString key;~ActiveScope(){files.erase(key);}};
struct Loader {
    Catalogue definitions;
    Identifiers ids;
    std::set<QString> active;
    void read(const QString& path,const QString& prefix,bool resources,bool idsOnly){
        QFile file(path);
        if(!file.open(QIODevice::ReadOnly|QIODevice::Text))throw MediaError(DiagnosticCode::DefinitionFile,{utf8(path)});
        const QFileInfo info(path);
        const auto canonical=resources?path:info.canonicalFilePath();
        const auto key=canonical.toCaseFolded();
        if(!active.insert(key).second)throw MediaError(DiagnosticCode::DefinitionCycle,{utf8(path)});
        const ActiveScope scope{active,key};
        QTextStream stream(&file);
        std::optional<DiskDefinition> disk;
        std::optional<std::size_t> parameters;
        std::int64_t lineNumber=0;
        while(!stream.atEnd()){
            ++lineNumber;
            const auto source=stream.readLine();
            const auto line=source.section(QChar::fromLatin1(Constants::Comment),0,0).trimmed();
            if(line.isEmpty())continue;
            const auto diskMatch=diskPattern().match(line);
            if(idsOnly&&diskMatch.hasMatch()){ids.insert(utf8((prefix+diskMatch.captured(Constants::DiskIdGroup)).toLower()));continue;}
            const auto imported=importPattern().match(line);
            if(imported.hasMatch()){
                const auto importedFile=imported.captured(Constants::ImportFileGroup);
                const auto next=resources?qtext(Constants::ResourcePrefix)+importedFile:info.dir().absoluteFilePath(importedFile);
                read(next,prefix+imported.captured(Constants::ImportPrefixGroup),resources,idsOnly);
                continue;
            }
            if(idsOnly)continue;
            if(diskMatch.hasMatch()){
                disk=DiskDefinition{utf8(prefix+diskMatch.captured(Constants::DiskIdGroup)),0,0,{}};
                parameters.reset();
            }else if(line.startsWith(qtext(Constants::TracksPrefix))){
                const auto words=line.split(wordsPattern(),Qt::SkipEmptyParts);
                if(!disk||words.size()<qsizetype(Constants::TrackWords))throw MediaError(DiagnosticCode::DefinitionSyntax,{utf8(path),std::to_string(lineNumber)});
                disk->tracks.push_back({utf8(words[Constants::SelectionWord]),utf8(words[Constants::CodecWord]),{}});
                parameters=disk->tracks.size()-1;
            }else if(line==qtext(Constants::End)){
                if(parameters)parameters.reset();
                else if(disk){const auto id=disk->id;if(!definitions.emplace(id,std::move(*disk)).second)throw MediaError(DiagnosticCode::DefinitionSyntax,{utf8(path),std::to_string(lineNumber)});disk.reset();}
            }else{
                const auto equal=line.indexOf(QChar::fromLatin1(Constants::Assignment));
                if(equal<0||!disk)continue;
                const auto name=utf8(line.left(equal).trimmed()),value=utf8(line.mid(equal+1).trimmed());
                if(parameters)disk->tracks[*parameters].parameters[name]=value;
                else if(name==Constants::Cylinders)disk->cylinders=parseNumber(name,value);
                else if(name==Constants::Heads)disk->heads=parseNumber(name,value);
            }
        }
    }
};
}
const Catalogue& embedded(){
    static const Catalogue catalogue=[](){Loader loader;loader.read(qtext(Constants::ResourcePrefix)+qtext(Constants::RootFile),{},true,false);return std::move(loader.definitions);}();
    return catalogue;
}
Catalogue rawDefinitions(std::span<const std::string> curatedIds){
    Identifiers curated(curatedIds.begin(),curatedIds.end());Catalogue result;
    for(const auto& [id,definition]:embedded())if(!curated.contains(id)&&!definition.tracks.empty()&&std::all_of(definition.tracks.begin(),definition.tracks.end(),[](const auto& track){return std::find(Constants::RawCodecs.begin(),Constants::RawCodecs.end(),track.codec)!=Constants::RawCodecs.end();}))result.emplace(id,definition);
    return result;
}
Identifiers readFileIds(const std::filesystem::path& path){
    const auto encoded=path.u8string();const auto input=QString::fromUtf8(reinterpret_cast<const char*>(encoded.data()),qsizetype(encoded.size()));
    if(input.trimmed().isEmpty())throw MediaError(DiagnosticCode::DefinitionFile,{utf8(input)});
    Loader loader;loader.read(QFileInfo(input).absoluteFilePath(),{},false,true);return std::move(loader.ids);
}
std::optional<std::pair<std::int32_t,std::int32_t>> dimensions(std::string_view format){
    const auto same=[&](auto id){return Primitives::ordinalIdentifierEqualsIgnoreCase(format,id);};
    if(same(Constants::Atari1440)||same(Constants::Mac1440))format=Constants::Ibm1440;
    else if(same(Constants::AppleSos))format=Constants::AppleProDos140;
    else if(same(Constants::AppleProDos800))format=Constants::Mac800;
    const auto found=embedded().find(format);
    if(found==embedded().end())return {};
    return std::pair{found->second.cylinders,found->second.heads};
}
}
