#include "Descriptor.h"
#include "../../../Primitives/Identifiers.h"
#include "../../../Sources/RandomAccessSources.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QRegularExpression>
#include <QFileInfo>
#include <QDir>
#include <QRandomGenerator>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Vmdk {
namespace {
QString text(std::string_view value){return QString::fromUtf8(value.data(),qsizetype(value.size()));}
QStringList lines(const QString& descriptor){auto values=descriptor.split(QRegularExpression(text(Constants::LinePattern)),Qt::SkipEmptyParts);for(auto& value:values)value=value.trimmed();return values;}
QString assigned(const QString& descriptor,std::string_view name){
    const auto prefix=text(name)+text(Constants::Assignment);for(const auto& line:lines(descriptor))if(line.startsWith(prefix,Qt::CaseInsensitive)){auto value=line.mid(prefix.size()).trimmed();while(value.startsWith(QChar(Constants::Quote)))value.remove(0,1);while(value.endsWith(QChar(Constants::Quote)))value.chop(1);return value;}throw MediaError(DiagnosticCode::VmdkAssignment,{std::string(name)});
}
struct Extent{QString access,type,name;std::int64_t sectors=0,offset=0;};
Extent singleExtent(const QString& descriptor){
    using namespace Constants;QStringList extents;
    for(const auto& line:lines(descriptor))if(line.startsWith(text(ReadWrite)+QChar(Space),Qt::CaseInsensitive)||line.startsWith(text(ReadOnly)+QChar(Space),Qt::CaseInsensitive)||line.startsWith(text(NoAccess)+QChar(Space),Qt::CaseInsensitive))extents.push_back(line);
    if(extents.size()!=1)throw MediaError(DiagnosticCode::VmdkExtentCount);const auto& line=extents.front();const auto first=line.indexOf(QChar(Quote)),second=first<0?-1:line.indexOf(QChar(Quote),first+1);
    if(first<0||second<0)throw MediaError(DiagnosticCode::VmdkExtentName);const auto prefix=line.left(first).split(QChar(Space),Qt::SkipEmptyParts);bool valid=false;
    const auto sectors=prefix.size()==ExtentPrefixFields?prefix[1].toLongLong(&valid):0;if(!valid||sectors<=0||std::uint64_t(sectors)>MaximumSigned/SectorSize)throw MediaError(DiagnosticCode::VmdkExtentCapacity);
    const auto suffix=line.mid(second+1).trimmed();std::int64_t offset=0;if(!suffix.isEmpty()){if(!QRegularExpression(text(DigitsPattern)).match(suffix).hasMatch())throw MediaError(DiagnosticCode::VmdkExtentOffset);offset=suffix.toLongLong(&valid);if(!valid||offset<0||std::uint64_t(offset)>MaximumSigned/SectorSize)throw MediaError(DiagnosticCode::VmdkExtentOffset);}
    return {prefix[0],prefix[ExtentTypeIndex],line.mid(first+1,second-first-1),sectors,offset};
}
}
void validateDescriptor(const QString& descriptor,std::string_view expected){
    const auto parent=text(Constants::ParentName)+text(Constants::Assignment);for(const auto& line:lines(descriptor))if(line.startsWith(parent,Qt::CaseInsensitive))throw MediaError(DiagnosticCode::VmdkParent);
    const auto type=assigned(descriptor,Constants::CreateTypeName);if(type.compare(text(expected),Qt::CaseInsensitive)!=0)throw MediaError(DiagnosticCode::VmdkType,{type.toUtf8().toStdString()});
}
std::shared_ptr<const MediaDocument> readDescriptor(const RecognitionContext& context,bool includeRanges,std::stop_token token){
    using namespace Constants;checkCancellation(token);if(context.length()>MaximumDescriptor)throw MediaError(DiagnosticCode::VmdkDescriptorSize);const auto bytes=context.readBytesAsync(token).get();const auto descriptor=Primitives::utf8Identifier(std::string_view(reinterpret_cast<const char*>(bytes.data()),bytes.size()));
    qsizetype start=0;const auto whitespace=text(InitialWhitespace);while(start<descriptor.size()&&(descriptor[start]==QChar(Bom)||whitespace.contains(descriptor[start])))++start;
    if(!QStringView(descriptor).mid(start).startsWith(text(DescriptorSignature)))throw MediaError(DiagnosticCode::VmdkSignature);validateDescriptor(descriptor,MonolithicFlat);const auto extent=singleExtent(descriptor);
    if(extent.access.compare(text(ReadWrite),Qt::CaseInsensitive)||extent.type.compare(text(Flat),Qt::CaseInsensitive))throw MediaError(DiagnosticCode::VmdkExtentMode);
    const auto sourceName=QString::fromUtf8(context.source()->primaryPath());const auto extentName=QDir(QFileInfo(sourceName).absolutePath()).absoluteFilePath(extent.name);const QFileInfo info(extentName);
    if(!info.isFile())throw MediaError(DiagnosticCode::VmdkExtentMissing,{extent.name.toUtf8().toStdString()});const auto capacity=extent.sectors*SectorSize,offset=extent.offset*SectorSize;
    if(offset>info.size()||capacity>info.size()-offset)throw MediaError(DiagnosticCode::VmdkExtentShort);const auto path=std::filesystem::path(info.absoluteFilePath().toStdU16String());
    std::vector<DataRange> ranges;if(includeRanges)ranges.emplace_back(0,capacity,DataRangeKind::Stored,fileSource(path),offset);else ranges.emplace_back(0,capacity,DataRangeKind::Unavailable);
    auto options=context.source()->options();const auto utf8=path.u8string();const std::string associated(utf8.begin(),utf8.end());std::vector<std::string> paths;for(const auto& candidate:options.associatedPaths)if(std::none_of(paths.begin(),paths.end(),[&](const auto& current){return Primitives::ordinalIdentifierEqualsIgnoreCase(current,candidate);}))paths.push_back(candidate);
    if(std::none_of(paths.begin(),paths.end(),[&](const auto& current){return Primitives::ordinalIdentifierEqualsIgnoreCase(current,associated);}))paths.push_back(associated);options.associatedPaths=std::move(paths);
    return createDocument(std::make_shared<const MediaSource>(std::move(options)),capacity,std::move(ranges),true,descriptor.toUtf8().toStdString());
}
std::vector<std::uint8_t> createDescriptor(const BlockImage& blocks,std::string_view type,std::string_view extentType,const std::filesystem::path& fileName,std::string_view suffix){
    using namespace Constants;const auto capacity=blocks.capacity()/SectorSize;const auto cylinders=blocks.geometry()?blocks.geometry()->cylinders():std::max(std::int64_t(1),capacity/(DefaultHeads*DefaultSectors));const auto heads=blocks.geometry()?blocks.geometry()->heads():DefaultHeads,sectors=blocks.geometry()?blocks.geometry()->sectorsPerTrack():DefaultSectors;
    std::uint32_t cid;do{cid=QRandomGenerator::global()->generate();}while(!cid||cid==MaximumField);
    auto result=text(DescriptorTemplate).arg(QString::number(cid,ContentIdRadix).rightJustified(ContentIdWidth,QChar(ContentIdPadding)),text(type),QString::number(capacity),text(extentType),QString::fromStdU16String(fileName.u16string()),text(suffix),QString::number(cylinders),QString::number(heads),QString::number(sectors)).toUtf8();
    return {reinterpret_cast<const std::uint8_t*>(result.constData()),reinterpret_cast<const std::uint8_t*>(result.constData())+result.size()};
}
}
