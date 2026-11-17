#include "MediaText.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/Translations/TranslationService.h>
#include <RetroMediaLab/Translations/TranslationConstants.h>
#include <Errors.h>
namespace RetroMediaLab {
QString mediaText(Translations::TranslationService& translations,const MediaEngine::MessageInfo& message) {
    auto result=QString::fromStdString(translations.text(message.resourceKey,message.translationDomain));
    for(std::size_t index=0;index<message.arguments.size();++index)result.replace(QString::fromLatin1(TranslationConstants::ArgumentPattern).arg(index),QString::fromStdString(message.arguments[index]));
    if(message.resourceKey==Errors::UnexpectedKey)result.replace(QString::fromLatin1(Errors::DetailParameter),message.arguments.empty()?QString{}:QString::fromStdString(message.arguments.front()));
    return result;
}
QString mediaErrorText(Translations::TranslationService& translations,const std::exception& error) {
    if(const auto* media=dynamic_cast<const MediaEngine::MediaError*>(&error))return mediaText(translations,{std::string(MediaEngine::diagnosticKey(media->diagnostic().code)),media->diagnostic().arguments,TranslationConstants::MediaEngine});
    const std::string key=error.what();const auto missing=TranslationConstants::MissingKeyOpen+key+TranslationConstants::MissingKeyClose;
    auto value=translations.text(key);if(value!=missing)return QString::fromStdString(value);
    value=translations.text(key,TranslationConstants::MediaEngine);if(value!=missing)return QString::fromStdString(value);
    return QString::fromStdString(Errors::render(translations.language(),key));
}
}
