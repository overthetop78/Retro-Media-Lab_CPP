#pragma once
#include "AppApi.h"
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QAction>
namespace RetroMediaLab {
class LocalizedAction final : public QAction {
public:
    RETROMEDIALAB_APP_API LocalizedAction(Translations::TranslationService&, const char* key, QObject* parent);
private:
    Translations::TranslationService::Subscription subscription_;
};
}
