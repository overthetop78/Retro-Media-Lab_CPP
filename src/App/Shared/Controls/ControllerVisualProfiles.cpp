#include "Shared/Controls/ControllerVisualProfiles.h"
#include "Logging/LogPaths.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QSaveFile>

namespace RetroMediaLab::ControllerVisualProfiles {
namespace {
using Model = ControllerArtwork::HostModel;
constexpr auto ProfileFile = QLatin1String("Controllers/visual-models.json");
constexpr auto ModelKey = QLatin1String("Model");
constexpr auto DisplayNameKey = QLatin1String("DisplayName");
constexpr int FirstModel = static_cast<int>(Model::GenericGamepad);
constexpr int LastModel = static_cast<int>(Model::ArcadeStick);
constexpr int InvalidModel = -1;

struct Store {
    QString path = QDir(applicationDataDirectory(QCoreApplication::applicationDirPath())).filePath(ProfileFile);
    QMutex mutex;
    QJsonObject document;

    Store() {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly)) document = QJsonDocument::fromJson(file.readAll()).object();
    }
};

Store& store() {
    static Store value;
    return value;
}

QString storedKey(const QJsonObject& document, const QString& id) {
    if (document.contains(id)) return id;
    for (auto item = document.constBegin(); item != document.constEnd(); ++item)
        if (item.key().compare(id, Qt::CaseInsensitive) == 0) return item.key();
    return id;
}

bool knownModel(int model) {
    return model >= FirstModel && model <= LastModel;
}
}

std::optional<Profile> profile(const QString& deviceId) {
    if (deviceId.trimmed().isEmpty()) return std::nullopt;
    auto& state = store();
    const QMutexLocker lock(&state.mutex);
    const auto value = state.document.value(storedKey(state.document, deviceId)).toObject();
    const int model = value.value(ModelKey).toInt(InvalidModel);
    if (!knownModel(model)) return std::nullopt;
    return Profile{static_cast<Model>(model), value.value(DisplayNameKey).toString()};
}

bool save(const QString& deviceId, const std::optional<Profile>& profile) {
    if (deviceId.trimmed().isEmpty()) return false;
    if (profile && !knownModel(static_cast<int>(profile->model))) return false;
    auto& state = store();
    const QMutexLocker lock(&state.mutex);
    auto updated = state.document;
    const auto key = storedKey(updated, deviceId);
    if (profile)
        updated.insert(key, QJsonObject{{ModelKey, static_cast<int>(profile->model)}, {DisplayNameKey, profile->displayName}});
    else if (updated.take(key).isUndefined())
        return true;
    if (!QDir().mkpath(QFileInfo(state.path).absolutePath())) return false;
    QSaveFile file(state.path);
    const auto bytes = QJsonDocument(updated).toJson();
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit()) return false;
    state.document = std::move(updated);
    return true;
}
}
