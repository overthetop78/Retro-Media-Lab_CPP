#include "Shared/Controls/ControllerAnalogProfiles.h"
#include "Logging/LogPaths.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QSaveFile>
#include <QCoreApplication>
#include <algorithm>
#include <cmath>

namespace RetroMediaLab::ControllerAnalogProfiles {
namespace {
constexpr auto ProfileFile = QLatin1String("Controllers/analog-dead-zones.json");
constexpr auto StickKey = QLatin1String("StickPercent");
constexpr auto TriggerKey = QLatin1String("TriggerPercent");
constexpr auto OuterKey = QLatin1String("OuterPercent");


constexpr float Percentage = 100.0f;
constexpr float MinimumUsableStickRange = 0.001f;
constexpr float TriggerTolerance = 0.0001f;

Profile normalize(Profile value) {
    value.stickPercent = std::clamp(value.stickPercent, 0, MaximumInnerPercent);
    value.triggerPercent = std::clamp(value.triggerPercent, 0, MaximumInnerPercent);
    value.outerPercent = std::clamp(value.outerPercent, 0, MaximumOuterPercent);
    return value;
}

struct Store {
    QString path = QDir(applicationDataDirectory(QCoreApplication::applicationDirPath())).filePath(ProfileFile);
    QMutex mutex;
    QHash<QString, Profile> persisted;
    QHash<QString, Profile> previews;

    Store() {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) return;
        const auto document = QJsonDocument::fromJson(file.readAll());
        const auto values = document.object();
        for (auto entry = values.constBegin(); entry != values.constEnd(); ++entry) {
            if (!entry->isObject()) continue;
            const auto value = entry->toObject();
            persisted.insert(entry.key().toCaseFolded(),
                normalize({value.value(StickKey).toInt(), value.value(TriggerKey).toInt(), value.value(OuterKey).toInt()}));
        }
    }
};

Store& store() {
    static Store value;
    return value;
}

void applyStick(float& x, float& y, Profile value) {
    x = std::clamp(x, -1.0f, 1.0f);
    y = std::clamp(y, -1.0f, 1.0f);
    const float magnitude = std::sqrt(x * x + y * y);
    const float inner = value.stickPercent / Percentage;
    if (magnitude <= inner) {
        x = y = 0;
        return;
    }
    const float outer = value.outerPercent / Percentage;
    const float usable = std::max(MinimumUsableStickRange, 1.0f - inner - outer);
    const float scale = std::clamp((magnitude - inner) / usable, 0.0f, 1.0f) / magnitude;
    x = std::clamp(x * scale, -1.0f, 1.0f);
    y = std::clamp(y * scale, -1.0f, 1.0f);
}

float applyTrigger(float value, int percent) {
    value = std::clamp(value, 0.0f, 1.0f);
    const float inner = percent / Percentage;
    return value <= inner + TriggerTolerance ? 0.0f : std::clamp((value - inner) / (1.0f - inner), 0.0f, 1.0f);
}
}

Profile profile(const QString& deviceId) {
    if (deviceId.trimmed().isEmpty()) return {};
    auto& state = store();
    const QMutexLocker lock(&state.mutex);
    const auto key = deviceId.toCaseFolded();
    return state.previews.value(key, state.persisted.value(key));
}

void preview(const QString& deviceId, Profile value) {
    if (deviceId.trimmed().isEmpty()) return;
    auto& state = store();
    const QMutexLocker lock(&state.mutex);
    state.previews.insert(deviceId.toCaseFolded(), normalize(value));
}

bool save(const QString& deviceId, Profile value) {
    if (deviceId.trimmed().isEmpty()) return false;
    auto& state = store();
    const QMutexLocker lock(&state.mutex);
    const auto key = deviceId.toCaseFolded();
    auto profiles = state.persisted;
    profiles.insert(key, normalize(value));
    QJsonObject document;
    for (auto entry = profiles.constBegin(); entry != profiles.constEnd(); ++entry) {
        document.insert(entry.key(), QJsonObject{
            {StickKey, entry->stickPercent}, {TriggerKey, entry->triggerPercent}, {OuterKey, entry->outerPercent}
        });
    }
    if (!QDir().mkpath(QFileInfo(state.path).absolutePath())) return false;
    const auto bytes = QJsonDocument(document).toJson();
    QSaveFile file(state.path);
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit()) return false;
    state.persisted = std::move(profiles);
    state.previews.remove(key);
    return true;
}

void apply(Platform::GamepadState& state, Profile value) {
    enum Axis { LeftX, LeftY, RightX, RightY, LeftTrigger, RightTrigger, AxisCount };
    if (state.axes.size() < AxisCount) return;
    value = normalize(value);
    auto& axes = state.axes;
    applyStick(axes[LeftX], axes[LeftY], value);
    applyStick(axes[RightX], axes[RightY], value);
    axes[LeftTrigger] = applyTrigger(axes[LeftTrigger], value.triggerPercent);
    axes[RightTrigger] = applyTrigger(axes[RightTrigger], value.triggerPercent);
    if (value.triggerPercent <= 0) return;
    state.buttons &= ~(Platform::LeftTriggerButton | Platform::RightTriggerButton);
    if (axes[LeftTrigger] > 0.0f) state.buttons |= Platform::LeftTriggerButton;
    if (axes[RightTrigger] > 0.0f) state.buttons |= Platform::RightTriggerButton;
}

QList<Platform::GamepadState> read(Platform::GamepadSource& source) {
    auto samples = source.read();
    for (auto& sample : samples) apply(sample, profile(sample.deviceId));
    return samples;
}
}
