#include "Media/MediaCommandConstants.h"
#include "LogSettings.h"
#include <ErrorKeys.h>
#include <stdexcept>
namespace RetroMediaLab {
const std::array<LogAction, LogConstants::ActionCount>& logActions() {
    static constexpr std::array<LogAction, LogConstants::ActionCount> definitions{{
        {LogConstants::Read, LogConstants::ReadLabel}, {LogConstants::Write, LogConstants::WriteLabel},
        {LogConstants::Convert, LogConstants::ConvertLabel}, {LogConstants::Erase, LogConstants::EraseLabel},
        {LogConstants::Clean, LogConstants::CleanLabel}, {LogConstants::Info, LogConstants::InfoLabel},
        {LogConstants::Bandwidth, LogConstants::BandwidthLabel}, {LogConstants::Rpm, LogConstants::RpmLabel},
        {LogConstants::Seek, LogConstants::SeekLabel}, {LogConstants::Pin, LogConstants::PinLabel},
        {LogConstants::Reset, LogConstants::ResetLabel}, {LogConstants::Delays, LogConstants::DelaysLabel},
        {LogConstants::Update, LogConstants::UpdateLabel}, {LogConstants::Align, LogConstants::AlignLabel}
    }};
    return definitions;
}
namespace {
QJsonObject encode(const ActionLogSettings& s) {
    return {{LogConstants::Enabled,s.enabled},{LogConstants::MaximumKilobytes,s.maximumKilobytes},{LogConstants::KeepArchives,s.keepArchives}};
}
ActionLogSettings decode(const QJsonObject& object, ActionLogSettings fallback) {
    const auto enabled=object.value(LogConstants::Enabled), size=object.value(LogConstants::MaximumKilobytes), archives=object.value(LogConstants::KeepArchives);
    if ((!enabled.isUndefined()&&!enabled.isBool())||(!archives.isUndefined()&&!archives.isBool())
        ||(!size.isUndefined()&&(!size.isDouble()||size.toInt(LogConstants::InvalidSize)<LogConstants::Unlimited||size.toDouble()!=size.toInt(LogConstants::InvalidSize))))
        throw std::invalid_argument(ErrorKeys::InvalidLoggingSettings);
    if(!enabled.isUndefined())fallback.enabled=enabled.toBool();
    if(!size.isUndefined())fallback.maximumKilobytes=size.toInt();
    if(!archives.isUndefined())fallback.keepArchives=archives.toBool();
    return fallback;
}
}
ActionLogSettings LoggingSettings::forAction(const QString& action) const { return actions.value(action==Infrastructure::MediaCommandConstants::ReadExplorer?QString::fromLatin1(LogConstants::Read):action.toLower(),defaults); }
void LoggingSettings::validate() const {
    if(defaults.maximumKilobytes<LogConstants::Unlimited)throw std::invalid_argument(ErrorKeys::InvalidLoggingSettings);
    for(auto it=actions.begin();it!=actions.end();++it)
        if(it.key().isEmpty()||it.key()!=it.key().toLower()||it.value().maximumKilobytes<LogConstants::Unlimited)throw std::invalid_argument(ErrorKeys::InvalidLoggingSettings);
}
QJsonObject LoggingSettings::toJson() const {
    validate();auto result=encode(defaults);QJsonObject perAction;
    for(auto it=actions.begin();it!=actions.end();++it)perAction.insert(it.key(),encode(it.value()));
    result.insert(LogConstants::Actions,perAction);return result;
}
LoggingSettings LoggingSettings::fromJson(const QJsonValue& value) {
    LoggingSettings result;if(value.isUndefined())return result;
    if(!value.isObject())throw std::invalid_argument(ErrorKeys::InvalidLoggingSettings);
    const auto object=value.toObject();result.defaults=decode(object,result.defaults);
    const auto entries=object.value(LogConstants::Actions);
    if(!entries.isUndefined()&&!entries.isObject())throw std::invalid_argument(ErrorKeys::InvalidLoggingSettings);
    const auto actions=entries.toObject();
    for(auto it=actions.begin();it!=actions.end();++it){
        const auto key=it.key().toLower();
        if(!it.value().isObject()||result.actions.contains(key))throw std::invalid_argument(ErrorKeys::InvalidLoggingSettings);
        result.actions.insert(key,decode(it.value().toObject(),result.defaults));
    }
    result.validate();return result;
}
}
