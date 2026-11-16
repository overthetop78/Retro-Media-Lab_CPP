#include "Catalogue.h"
#include "Constants.h"
#include "../Definitions/DiskDefinitions.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <algorithm>

namespace RetroMediaLab::MediaEngine {
namespace {
QString qtext(std::string_view text){return QString::fromUtf8(text.data(),qsizetype(text.size()));}
[[noreturn]] void invalid(){throw MediaError(DiagnosticCode::FormatCatalogData,{std::string(CatalogConstants::Resource)});}
QJsonValue field(const QJsonObject& object,std::string_view name){return object.value(qtext(name));}
std::string string(const QJsonValue& value){if(!value.isString()||value.toString().isEmpty())invalid();return value.toString().toUtf8().toStdString();}
std::string string(const QJsonObject& object,std::string_view name){return string(field(object,name));}
bool boolean(const QJsonObject& object,std::string_view name){const auto value=field(object,name);if(!value.isBool())invalid();return value.toBool();}
template<class Enum> Enum enumeration(const QJsonObject& object,std::string_view name,Enum maximum){
    const auto value=field(object,name);if(!value.isDouble())invalid();const auto number=value.toInt(-1);
    if(number<0||number>int(maximum)||double(number)!=value.toDouble())invalid();return Enum(number);
}
std::vector<DiskFormat> load(){
    QFile file(qtext(CatalogConstants::Resource));if(!file.open(QIODevice::ReadOnly))invalid();
    QJsonParseError error;const auto document=QJsonDocument::fromJson(file.readAll(),&error);
    if(error.error!=QJsonParseError::NoError||!document.isArray())invalid();
    std::vector<DiskFormat> result;Definitions::Identifiers ids;
    for(const auto& entry:document.array()){
        if(!entry.isObject())invalid();const auto object=entry.toObject();DiskFormat format;
        format.id=string(object,CatalogConstants::Id);if(!ids.insert(format.id).second)invalid();
        format.family=string(object,CatalogConstants::Family);format.familyNameKey=string(object,CatalogConstants::FamilyKey);format.displayNameKey=string(object,CatalogConstants::DisplayKey);
        const auto extensions=field(object,CatalogConstants::Extensions);if(!extensions.isArray())invalid();
        for(const auto& value:extensions.toArray()){
            if(!value.isObject())invalid();const auto extension=value.toObject();
            format.extensions.push_back({string(extension,CatalogConstants::Extension),string(extension,CatalogConstants::DisplayKey),boolean(extension,CatalogConstants::Default)});
        }
        const auto sources=field(object,CatalogConstants::Sources);if(!sources.isNull()){
            if(!sources.isArray())invalid();format.compatibleSourceExtensions.emplace();
            for(const auto& source:sources.toArray())format.compatibleSourceExtensions->push_back(string(source));
        }
        const auto tag=field(object,CatalogConstants::Tag);if(!tag.isNull())format.tag=string(tag);
        format.isCommon=boolean(object,CatalogConstants::Common);
        format.supportsPhysicalRead=boolean(object,CatalogConstants::PhysicalRead);format.supportsPhysicalWrite=boolean(object,CatalogConstants::PhysicalWrite);
        format.formFactor=enumeration(object,CatalogConstants::FormFactor,FloppyFormFactor::EightInch);
        format.density=enumeration(object,CatalogConstants::Density,FloppyDensity::ExtendedDensity);
        result.push_back(std::move(format));
    }
    if(result.empty())invalid();return result;
}
}
std::span<const DiskFormat> imageFormats(){static const auto formats=load();return formats;}
const DiskFormat* findImageFormat(std::string_view id){const auto formats=imageFormats();const auto found=std::find_if(formats.begin(),formats.end(),[&](const auto& item){return Primitives::ordinalIdentifierEqualsIgnoreCase(item.id,id);});return found==formats.end()?nullptr:&*found;}
std::vector<std::string> imageFormatIds(){std::vector<std::string> result;for(const auto& format:imageFormats())result.push_back(format.id);return result;}
std::vector<DiskFormat> compatibleImageFormats(std::span<const DiskFormat> formats,std::string_view extension){
    auto normalized=qtext(extension).toLower();if(!normalized.startsWith(QChar::fromLatin1(CatalogConstants::ExtensionSeparator)))normalized.prepend(QChar::fromLatin1(CatalogConstants::ExtensionSeparator));
    const auto value=normalized.toUtf8().toStdString();std::vector<DiskFormat> result;
    for(const auto& format:formats)if(format.compatibleSourceExtensions&&std::any_of(format.compatibleSourceExtensions->begin(),format.compatibleSourceExtensions->end(),[&](const auto& source){return Primitives::ordinalIdentifierEqualsIgnoreCase(source,value);}))result.push_back(format);
    return result;
}
std::vector<DiskFormat> compatibleImageFormats(std::string_view extension){return compatibleImageFormats(imageFormats(),extension);}
}
