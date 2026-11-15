#include "ImageDisk.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <algorithm>
#include <cmath>
#include <limits>
namespace RetroMediaLab::MediaEngine::ImageDisk {
namespace {
using Path=std::vector<std::string>;
QString key(std::string_view name){return QString::fromLatin1(name.data(),qsizetype(name.size()));}
[[noreturn]] void invalid(){throw MediaError(DiagnosticCode::ImdMetadata);}
// QJson normalizes number syntax. Retain integer lexemes at each path so GetByte/GetInt32
// keeps the C# distinction between 1, 1.0 and 1e0, including duplicate-property precedence.
class IntegerSyntax {
 std::string_view text_;std::size_t position_=0;std::map<Path,bool> integers_;
 void whitespace(){while(position_<text_.size()&&Constants::JsonWhitespace.find(text_[position_])!=std::string_view::npos)++position_;}
 void erase(const Path& path){auto i=integers_.lower_bound(path);while(i!=integers_.end()&&i->first.size()>=path.size()&&std::equal(path.begin(),path.end(),i->first.begin()))i=integers_.erase(i);}
 std::string string(){using namespace Constants;const auto start=position_++;bool escaped=false;while(position_<text_.size()){const auto value=text_[position_++];if(value==Escape){escaped=true;if(position_>=text_.size())invalid();++position_;}else if(value==Quote){const auto token=text_.substr(start,position_-start);if(!escaped)return std::string(token.substr(1,token.size()-JsonQuotePairLength));const std::string wrapped=std::string(1,ArrayOpen)+std::string(token)+std::string(1,ArrayClose);const auto parsed=QJsonDocument::fromJson(QByteArray::fromStdString(wrapped));if(!parsed.isArray()||parsed.array().size()!=1)invalid();return parsed.array().first().toString().toStdString();}}invalid();}
 void value(Path path,std::size_t depth){using namespace Constants;whitespace();if(position_>=text_.size())invalid();erase(path);const auto first=text_[position_];if(first==ObjectOpen||first==ArrayOpen){if(depth>=JsonMaxDepth)invalid();++position_;whitespace();const auto close=first==ObjectOpen?ObjectClose:ArrayClose;if(position_<text_.size()&&text_[position_]==close){++position_;return;}std::size_t index=0;for(;;){Path child=path;if(first==ObjectOpen){if(position_>=text_.size()||text_[position_]!=Quote)invalid();child.push_back(string());whitespace();if(position_>=text_.size()||text_[position_++]!=Colon)invalid();}else child.push_back(std::to_string(index++));value(std::move(child),depth+1);whitespace();if(position_>=text_.size())invalid();const auto ending=text_[position_++];if(ending==close)return;if(ending!=Comma)invalid();whitespace();}}
  if(first==Quote){string();return;}const auto start=position_;while(position_<text_.size()&&text_[position_]!=Comma&&text_[position_]!=ObjectClose&&text_[position_]!=ArrayClose&&JsonWhitespace.find(text_[position_])==std::string_view::npos)++position_;const auto token=text_.substr(start,position_-start);integers_[std::move(path)]=token.find(Dot)==std::string_view::npos&&token.find(ExponentLower)==std::string_view::npos&&token.find(ExponentUpper)==std::string_view::npos;
 }
public:
 explicit IntegerSyntax(std::string_view text):text_(text){value({},0);whitespace();if(position_!=text_.size())invalid();}
 bool integer(const Path& p)const{const auto found=integers_.find(p);return found!=integers_.end()&&found->second;}
};
const QJsonValue property(const QJsonObject& object,std::string_view name){const auto k=key(name);if(!object.contains(k))invalid();return object.value(k);}
QJsonArray array(const QJsonValue& value){if(!value.isArray())invalid();return value.toArray();}
QJsonObject object(const QJsonValue& value){if(!value.isObject())invalid();return value.toObject();}
std::int32_t integer(const QJsonObject& object,std::string_view name,const Path& parent,const IntegerSyntax& syntax,bool byte){auto path=parent;path.push_back(std::string(name));const auto value=property(object,name);const auto d=value.toDouble(std::numeric_limits<double>::quiet_NaN());if(!value.isDouble()||!syntax.integer(path)||!std::isfinite(d)||d!=std::trunc(d)||d<(byte?0:std::numeric_limits<std::int32_t>::min())||d>(byte?Constants::MaxByte:std::numeric_limits<std::int32_t>::max()))invalid();return std::int32_t(d);}
}
Metadata metadata(const Image& image){using namespace Constants;QJsonArray tracks;for(const auto& t:image.tracks){QJsonArray sectors;for(const auto&s:t.sectors)sectors.append(QJsonObject{{key(CylinderKey),s.cylinder},{key(HeadKey),s.head},{key(NumberKey),s.number},{key(SizeKey),s.size},{key(RecordTypeKey),std::uint8_t(s.recordType)}});tracks.append(QJsonObject{{key(ModeKey),std::uint8_t(t.mode)},{key(CylinderKey),t.cylinder},{key(HeadKey),t.head},{key(SectorsKey),sectors}});}QJsonObject root{{key(CommentKey),QString::fromUtf8(image.comment)},{key(TracksKey),tracks}};return {{std::string(MetadataKey),QJsonDocument(root).toJson(QJsonDocument::Compact).toStdString()}};}
std::optional<Image> restore(std::shared_ptr<const MediaFileSystems::SectorImage> source,const Metadata& data){using namespace Constants;if(!source)invalid();const auto found=data.find(std::string(MetadataKey));if(found==data.end())return {};QJsonParseError error;const auto document=QJsonDocument::fromJson(QByteArray::fromStdString(found->second),&error);if(error.error!=QJsonParseError::NoError||!document.isObject())invalid();const IntegerSyntax syntax(found->second);const auto root=document.object();const auto text=property(root,CommentKey);if(!text.isString()&&!text.isNull())invalid();Image result;result.comment=text.toString().toStdString();result.sectorImage=source;std::map<std::uint64_t,std::shared_ptr<const MediaFileSystems::SectorBlock>> blocks;source->visitAvailableBlocks([&](auto logical){blocks.emplace(logical,source->block(logical));return true;});std::uint64_t logical=0;const auto tracks=array(property(root,TracksKey));
 for(qsizetype ti=0;ti<tracks.size();++ti){const auto element=object(tracks[ti]);const Path path{std::string(TracksKey),std::to_string(ti)};const auto mode=integer(element,ModeKey,path,syntax,true);if(mode>MaxMode)throw MediaError(DiagnosticCode::ImdMetadataMode);Track track;track.mode=Mode(mode);track.cylinder=std::uint8_t(integer(element,CylinderKey,path,syntax,true));track.head=std::uint8_t(integer(element,HeadKey,path,syntax,true));std::vector<std::shared_ptr<const MediaFileSystems::SectorBlock>> trackBlocks;for(const auto&[number,b]:blocks){(void)number;if(b->address.cylinder==track.cylinder&&b->address.head==track.head)trackBlocks.push_back(b);}const auto sectors=array(property(element,SectorsKey));
  for(qsizetype si=0;si<sectors.size();++si){const auto s=object(sectors[si]);auto sectorPath=path;sectorPath.push_back(std::string(SectorsKey));sectorPath.push_back(std::to_string(si));const auto kind=integer(s,RecordTypeKey,sectorPath,syntax,true);if(kind>MaxRecord)throw MediaError(DiagnosticCode::ImdMetadataRecord);Sector sector;sector.recordType=RecordType(kind);sector.size=integer(s,SizeKey,sectorPath,syntax,false);if(sector.size<=0)throw MediaError(DiagnosticCode::ImdMetadataSize);
   if(hasData(sector.recordType)){std::shared_ptr<const MediaFileSystems::SectorBlock> b;const auto recorded=std::find_if(trackBlocks.begin(),trackBlocks.end(),[&](const auto& value){return value->recordedTrackOrder&&*value->recordedTrackOrder==si;});if(recorded!=trackBlocks.end())b=*recorded;else{const auto entry=blocks.find(logical);if(entry!=blocks.end())b=entry->second;}if(!b||b->data.size()!=std::size_t(sector.size))throw MediaError(DiagnosticCode::ImdMetadataBlock,{std::to_string(logical)});sector.data=b->data;const auto repeated=std::all_of(sector.data.begin(),sector.data.end(),[&](auto v){return v==sector.data.front();});sector.recordType=recordType(compressed(sector.recordType)&&repeated,b->integrityValid!=false,b->deletedDataMark.value_or(deleted(sector.recordType)));}else sector.data.resize(std::size_t(sector.size));
   sector.cylinder=std::uint8_t(integer(s,CylinderKey,sectorPath,syntax,true));sector.head=std::uint8_t(integer(s,HeadKey,sectorPath,syntax,true));sector.number=std::uint8_t(integer(s,NumberKey,sectorPath,syntax,true));track.sectors.push_back(std::move(sector));++logical;}result.tracks.push_back(std::move(track));}
 if(logical!=source->blockCount())throw MediaError(DiagnosticCode::ImdMetadataCount);return result;
}
}
