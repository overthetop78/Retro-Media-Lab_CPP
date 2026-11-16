#include <RetroMediaLab/MediaEngine/ConversionPlanning.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include "../WriterRegistry.h"
#include "../../Formats/Scp/Constants.h"
#include "../../Formats/Hfe/Constants.h"
#include "../../Primitives/Identifiers.h"
#include <QDateTime>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace {
std::filesystem::path utf8Path(std::string_view value){return std::filesystem::path(std::u8string(value.begin(),value.end()));}
QString text(std::string_view value){return Primitives::utf8Identifier(value);}
std::string utf8(const std::filesystem::path& path){const auto value=path.u8string();return {value.begin(),value.end()};}
}
ConversionFidelityLevel rebuiltConversionFidelity(std::string_view extension){
 const auto lower=text(extension).toLower().toUtf8().toStdString();return lower==Scp::Constants::Extension||lower==Hfe::Constants::Extension?ConversionFidelityLevel::ReconstructedTracks:ConversionFidelityLevel::SectorData;
}
ConversionFidelityLevel conversionFidelity(std::string_view source,std::string_view target){
 const auto a=normalizeExtension(source),b=normalizeExtension(target);
 if((a==b&&(a==Scp::Constants::Extension||a==Hfe::Constants::Extension))||(a==Hfe::Constants::Extension&&b==Scp::Constants::Extension))return ConversionFidelityLevel::PreservedFlux;
 return rebuiltConversionFidelity(b);
}
std::string formatConversionTag(std::string_view pattern,const DiskFormat& format,std::string_view extension,std::string_view name,std::chrono::system_clock::time_point timestamp){
 namespace C=ConversionPlanningConstants;auto result=text(pattern);
 if(result.trimmed().isEmpty()||!std::any_of(C::SupportedTokens.begin(),C::SupportedTokens.end(),[&](auto token){return result.contains(text(token),Qt::CaseInsensitive);}))throw MediaError(DiagnosticCode::ConversionTagPattern);
 auto legacy=format.tag?text(*format.tag):text(format.id).toUpper().replace(QChar::fromLatin1(C::IdSeparator),QChar::fromLatin1(C::TagSeparator));
 const auto split=legacy.indexOf(QChar::fromLatin1(C::TagSeparator));const auto family=split<0?legacy:legacy.left(split);
 const auto disk=split<0?text(format.id).section(QChar::fromLatin1(C::IdSeparator),-1).toUpper():legacy.mid(split+1);
 auto ext=text(extension);while(ext.startsWith(QChar::fromLatin1(C::IdSeparator)))ext.remove(0,1);
 result.replace(text(C::Tag),legacy,Qt::CaseInsensitive).replace(text(C::Family),family,Qt::CaseInsensitive).replace(text(C::Format),disk,Qt::CaseInsensitive).replace(text(C::Extension),ext.toUpper(),Qt::CaseInsensitive).replace(text(C::Name),text(name),Qt::CaseInsensitive);
 const auto time=QDateTime::fromMSecsSinceEpoch(std::chrono::duration_cast<std::chrono::milliseconds>(timestamp.time_since_epoch()).count()).toLocalTime();
 for(std::size_t i=0;i<C::DateTimeTokens.size();++i)result.replace(text(C::DateTimeTokens[i]),time.toString(text(C::DateTimePatterns[i])),Qt::CaseInsensitive);
 return result.toUtf8().toStdString();
}
ConversionOutput::ConversionOutput(std::string format,std::string suffix,std::filesystem::path path,bool implicit,std::optional<ConversionFidelityLevel> level):formatId(std::move(format)),extension(std::move(suffix)),outputPath(std::move(path)),usesImplicitExtension(implicit),fidelity(level.value_or(rebuiltConversionFidelity(extension))){}
ConversionPlanner::ConversionPlanner():formats_(imageFormats().begin(),imageFormats().end()){}
ConversionPlanner::ConversionPlanner(std::vector<DiskFormat> formats):formats_(std::move(formats)){}
std::vector<DiskFormat> ConversionPlanner::outputsForSource(std::string_view extension,std::optional<std::string_view> detection,bool requiresChoice)const{
 if(Primitives::isBlankIdentifier(extension))return formats_;
 const auto normalized=normalizeExtension(extension);
 if(normalized==Scp::Constants::Extension||normalized==Hfe::Constants::Extension||!detection||requiresChoice)return compatibleImageFormats(formats_,normalized);
 std::vector<DiskFormat> result;for(const auto& format:formats_)if(format.id==*detection||format.id==Scp::Constants::FormatId)result.push_back(format);return result;
}
std::vector<ConversionOutput> ConversionPlanner::plan(const std::filesystem::path& source,std::string base,std::span<const ConversionSelection> selections,bool addTags,std::string_view pattern)const{
 namespace C=ConversionPlanningConstants;const auto extension=utf8(source.extension());const auto compatible=compatibleImageFormats(formats_,extension);std::vector<ConversionOutput> outputs;
 for(const auto& selection:selections){
  const auto found=std::find_if(compatible.begin(),compatible.end(),[&](const auto& value){return value.id==selection.formatId;});
  if(found==compatible.end())throw MediaError(DiagnosticCode::ConversionIncompatibleFormat,{selection.formatId,extension});
  const auto& format=*found;std::vector<std::string> requested=selection.explicitExtensions;
  if(requested.empty())for(const auto& item:format.extensions)if(item.isDefault)requested.push_back(item.extension);
  for(const auto& wanted:requested){
   const auto item=std::find_if(format.extensions.begin(),format.extensions.end(),[&](const auto& value){return Primitives::ordinalIdentifierEqualsIgnoreCase(value.extension,wanted);});
   if(item==format.extensions.end())throw MediaError(DiagnosticCode::ConversionIncompatibleExtension,{wanted,format.id});
   const auto tag=addTags?formatConversionTag(pattern,format,item->extension,base,std::chrono::system_clock::now()):std::string{};
   const auto file=!addTags?base:text(pattern).contains(text(C::Name),Qt::CaseInsensitive)?tag:tag+base;
   const auto output=source.parent_path()/utf8Path(file+item->extension);
   outputs.push_back({format.id,item->extension,output,selection.explicitExtensions.empty(),conversionFidelity(extension,item->extension)});
  }
 }
 for(std::size_t i=0;i<outputs.size();++i)for(std::size_t previous=0;previous<i;++previous)if(Primitives::ordinalIdentifierEqualsIgnoreCase(utf8(outputs[i].outputPath),utf8(outputs[previous].outputPath)))throw MediaError(DiagnosticCode::ConversionOutputCollision,{utf8(outputs[i].outputPath)});
 return outputs;
}
}
