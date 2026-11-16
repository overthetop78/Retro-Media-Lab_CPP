#include "Container.h"
#include "../../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDir>
#include <charconv>
#include <unordered_set>
namespace RetroMediaLab::MediaEngine::Gdi {
namespace {
using namespace Constants;
QString text(std::string_view s){return Primitives::utf8Identifier(s);}
QString pathText(const std::filesystem::path& p){const auto s=p.u8string();return text(std::string_view(reinterpret_cast<const char*>(s.data()),s.size()));}
std::filesystem::path nativePath(const QString& value){const auto s=value.toUtf8();return std::filesystem::path(std::u8string(reinterpret_cast<const char8_t*>(s.data()),std::size_t(s.size())));}
std::uint64_t number(const QString& value,std::uint64_t maximum,bool positive){const auto s=value.toUtf8();std::uint64_t result=0;const auto parsed=std::from_chars(s.data(),s.data()+s.size(),result);if(s.isEmpty()||parsed.ec!=std::errc()||parsed.ptr!=s.data()+s.size()||result>maximum||(positive&&!result))throw MediaError(DiagnosticCode::GdiDescriptor);return result;}
QStringList tokenize(const QString& line){QStringList result;QString buffer;bool quoted=false;for(auto c:line){if(c==QChar(Quote)){quoted=!quoted;continue;}if(c.isSpace()&&!quoted){if(!buffer.isEmpty()){result.push_back(buffer);buffer.clear();}continue;}buffer+=c;}if(quoted)throw MediaError(DiagnosticCode::GdiDescriptor);if(!buffer.isEmpty())result.push_back(buffer);return result;}
}
Descriptor readDescriptor(const std::filesystem::path& path,std::stop_token token){
    using namespace Constants;checkCancellation(token);QFile file(pathText(path));if(!file.open(QIODevice::ReadOnly))throw MediaError(DiagnosticCode::GdiDescriptorIo,{file.fileName().toUtf8().toStdString()});QTextStream input(&file);QStringList lines;
    while(!input.atEnd()){checkCancellation(token);const auto line=input.readLine().trimmed();if(!line.isEmpty()&&!line.startsWith(QChar(Comment)))lines.push_back(line);}if(file.error()!=QFileDevice::NoError)throw MediaError(DiagnosticCode::GdiDescriptorIo,{file.fileName().toUtf8().toStdString()});
    if(lines.size()<MinimumLines)throw MediaError(DiagnosticCode::GdiDescriptor);const auto count=number(lines[0],MaximumInt,true);if(std::uint64_t(lines.size()-1)!=count)throw MediaError(DiagnosticCode::GdiDescriptor);Descriptor result;result.reserve(std::size_t(count));std::unordered_set<int> numbers;std::int64_t previous=-1;
    for(std::size_t i=0;i<std::size_t(count);++i){checkCancellation(token);const auto fields=tokenize(lines[qsizetype(i)+1]);if(fields.size()!=FieldCount)throw MediaError(DiagnosticCode::GdiDescriptor);TrackDeclaration track;track.number=int(number(fields[NumberField],MaximumInt,true));track.lba=std::int64_t(number(fields[LbaField],MaximumSigned,false));track.control=int(number(fields[ControlField],MaximumInt,false));track.sectorSize=int(number(fields[SizeField],MaximumInt,true));track.fileName=fields[FileField].toUtf8().toStdString();track.fileOffset=std::int64_t(number(fields[OffsetField],MaximumSigned,false));
        if(!numbers.insert(track.number).second||track.lba<=previous)throw MediaError(DiagnosticCode::GdiDescriptor);if((track.control!=AudioControl&&track.control!=DataControl)||(track.sectorSize!=DataBytes&&track.sectorSize!=RawBytes))throw MediaError(DiagnosticCode::GdiProfile);if(track.fileName.empty()||QDir::isAbsolutePath(fields[FileField]))throw MediaError(DiagnosticCode::GdiPath,{track.fileName});previous=track.lba;result.push_back(std::move(track));}
    return result;
}
std::filesystem::path resolveDataPath(const std::filesystem::path& descriptor,std::string_view name){
    using namespace Constants;const auto directory=QFileInfo(pathText(descriptor)).absolutePath();auto root=QDir::cleanPath(directory);if(!root.endsWith(QChar(PathSeparator)))root+=QChar(PathSeparator);const auto resolved=QDir::cleanPath(QFileInfo(QDir(directory).filePath(text(name))).absoluteFilePath());
    if(!Primitives::ordinalIdentifierStartsWithIgnoreCase(resolved.toUtf8().toStdString(),root.toUtf8().toStdString()))throw MediaError(DiagnosticCode::GdiPath,{std::string(name)});return nativePath(resolved);
}
std::vector<std::filesystem::path> associatedPaths(const std::filesystem::path& descriptor,const Descriptor& tracks){std::vector<std::filesystem::path> result;for(const auto& track:tracks){auto path=resolveDataPath(descriptor,track.fileName);if(!QFileInfo(pathText(path)).isFile())throw MediaError(DiagnosticCode::GdiPath,{track.fileName});result.push_back(std::move(path));}return result;}
}
