#include "LocalizedAction.h"
namespace RetroMediaLab {
LocalizedAction::LocalizedAction(Translations::TranslationService& translations, const char* key, QObject* parent)
    : QAction(parent) {
    setObjectName(key);
    const auto refresh = [this, &translations, key = std::string(key)] {
        setText(QString::fromStdString(translations.text(key)));
    };
    refresh();
    subscription_ = translations.subscribe(refresh);
}
}
