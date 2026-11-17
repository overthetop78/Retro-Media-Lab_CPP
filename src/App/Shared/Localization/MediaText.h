#pragma once
#include "AppApi.h"
#include <RetroMediaLab/MediaEngine/MessageInfo.h>
#include <QString>
#include <exception>
namespace RetroMediaLab {
namespace Translations {class TranslationService;}
RETROMEDIALAB_APP_API QString mediaText(Translations::TranslationService&,const MediaEngine::MessageInfo&);
RETROMEDIALAB_APP_API QString mediaErrorText(Translations::TranslationService&,const std::exception&);
}
