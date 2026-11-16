#include "Catalogue.h"
#include "Constants.h"
#include "../Definitions/DiskDefinitions.h"
#include "../Definitions/Constants.h"
#include "../Scp/Constants.h"
#include "../Hfe/Constants.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/GwFormatCapabilities.h>
#include <RetroMediaLab/MediaEngine/BuiltInDiskDefinitions.h>
#include <QRegularExpression>
#include <QStringList>
#include <algorithm>

namespace RetroMediaLab::MediaEngine {
namespace {
namespace C=CatalogConstants;
namespace D=Definitions::Constants;
using Ids=Definitions::Identifiers;
QString qtext(std::string_view text){return QString::fromUtf8(text.data(),qsizetype(text.size()));}
std::string utf8(const QString& text){return text.toUtf8().toStdString();}
QString section(const QString& text,std::string_view startMarker,std::optional<std::string_view> endMarker){
    auto start=text.indexOf(qtext(startMarker),0,Qt::CaseInsensitive);if(start<0)return {};
    start+=qsizetype(startMarker.size());const auto end=endMarker?text.indexOf(qtext(*endMarker),start,Qt::CaseInsensitive):text.size();
    return text.mid(start,end<0?-1:end-start);
}
std::vector<std::string> matches(const QString& text,std::string_view pattern){
    const QRegularExpression regex(qtext(pattern),QRegularExpression::CaseInsensitiveOption);Ids values;auto matches=regex.globalMatch(text);
    while(matches.hasNext())values.insert(utf8(matches.next().captured().toLower()));
    return {values.begin(),values.end()};
}
std::string normalizedExtension(std::string_view value){auto text=qtext(value).toLower();if(!text.startsWith(QChar::fromLatin1(C::ExtensionSeparator)))text.prepend(QChar::fromLatin1(C::ExtensionSeparator));return utf8(text);}
QString friendlyToken(const QString& token){
    const auto lower=token.toLower();
    for(const auto& [key,value]:C::FriendlyTokens)if(lower==qtext(key))return qtext(value);
    const QRegularExpression separator(qtext(C::WordSeparatorPattern));
    const auto words=QString(token).replace(QChar::fromLatin1(C::Underscore),QChar::fromLatin1(C::Hyphen)).split(separator,Qt::SkipEmptyParts);QStringList formatted;
    for(const auto& word:words){auto value=word.toLower();if(!value.isEmpty())value.replace(0,1,value.left(1).toUpper());formatted.push_back(value);}
    return formatted.join(qtext(C::WordSeparator));
}
ImageExtension extension(std::string name,bool isDefault){
    auto label=qtext(name);while(label.startsWith(QChar::fromLatin1(C::ExtensionSeparator)))label.remove(0,1);
    return {std::move(name),{},isDefault,utf8(label.toUpper())};
}
DiskFormat discovered(const std::string& id,const Ids& supported){
    const auto parts=qtext(id).split(QChar::fromLatin1(C::ExtensionSeparator));const auto family=friendlyToken(parts.front());
    QStringList details;for(qsizetype index=1;index<parts.size();++index)details.push_back(friendlyToken(parts[index]));
    DiskFormat result;result.id=id;result.family=utf8(family);result.technicalDisplayName=utf8(parts.size()>1?family+qtext(C::NameSeparator)+details.join(qtext(C::DetailSeparator)):family);result.isCommon=false;
    if(supported.contains(C::ImgExtension))result.extensions.push_back(extension(std::string(C::ImgExtension),true));
    else if(supported.contains(Scp::Constants::Extension))result.extensions.push_back(extension(std::string(Scp::Constants::Extension),true));
    else if(!supported.empty())result.extensions.push_back(extension(*supported.begin(),true));
    const auto found=Definitions::embedded().find(id);
    if(found!=Definitions::embedded().end()&&std::all_of(found->second.tracks.begin(),found->second.tracks.end(),[](const auto& track){return track.codec==D::BitcellCodec;})){
        result.extensions.clear();
        for(const auto suffix:{Scp::Constants::Extension,Hfe::Constants::Extension})if(supported.contains(suffix))result.extensions.push_back(extension(std::string(suffix),result.extensions.empty()));
    }
    Ids sources;for(const auto& suffix:supported)sources.insert(normalizedExtension(suffix));result.compatibleSourceExtensions.emplace(sources.begin(),sources.end());
    auto tag=qtext(id).toUpper().replace(QRegularExpression(qtext(C::TagPattern)),QString(QChar::fromLatin1(C::Hyphen)));
    while(tag.startsWith(QChar::fromLatin1(C::Hyphen)))tag.remove(0,1);
    while(tag.endsWith(QChar::fromLatin1(C::Hyphen)))tag.chop(1);
    result.tag=utf8(tag);
    // Discovered names and suffix labels are technical invariants, not localized UI captions.
    return result;
}
}
GwFormatCapabilities parseGwReadHelp(std::string_view output){
    const auto text=qtext(output);if(text.trimmed().isEmpty())return {};
    return {matches(section(text,C::FormatSection,C::SuffixSection),C::FormatPattern),matches(section(text,C::SuffixSection,{}),C::ExtensionPattern)};
}
std::optional<std::string> gwFormatArgument(std::string_view id){
    const auto same=[&](auto known){return Primitives::ordinalIdentifierEqualsIgnoreCase(id,known);};
    if(Primitives::isBlankIdentifier(id)||same(Scp::Constants::FormatId)||same(Hfe::Constants::FormatId))return {};
    if(same(D::Atari1440)||same(D::Mac1440))return std::string(D::Ibm1440);
    if(same(D::AppleSos))return std::string(D::AppleProDos140);
    if(same(D::AppleProDos800))return std::string(D::Mac800);
    return std::string(id);
}
std::vector<DiskFormat> capabilityAwareImageFormats(std::span<const DiskFormat> curated,const GwFormatCapabilities& capabilities){
    std::vector<DiskFormat> result(curated.begin(),curated.end());if(!capabilities.isKnown())return result;
    const Ids ids(capabilities.formatIds.begin(),capabilities.formatIds.end()),extensions(capabilities.imageExtensions.begin(),capabilities.imageExtensions.end());Ids known;
    for(auto& format:result){
        known.insert(format.id);if(!format.supportsPhysicalRead&&!format.supportsPhysicalWrite)continue;
        const auto argument=gwFormatArgument(format.id);
        const bool supported=format.id==Scp::Constants::FormatId||format.id==Hfe::Constants::FormatId||supportsBuiltInDiskDefinition(format.id)||(argument&&ids.contains(*argument));
        format.supportsPhysicalRead&=supported;format.supportsPhysicalWrite&=supported;
    }
    for(const auto& id:ids)if(!known.contains(id)){auto format=discovered(id,extensions);if(!format.extensions.empty())result.push_back(std::move(format));}
    return result;
}
}
