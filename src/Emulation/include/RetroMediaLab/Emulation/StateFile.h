#pragma once
#include "Host.h"
#include <QJsonValue>
#include <optional>

namespace RetroMediaLab::Emulation {
struct StateIdentity {
    QJsonValue core;
    QString coreName;
    QString coreVersion;
    QString coreSha256;
    QJsonValue model;
    QString configurationSha256;
    QString contentSha256;
};

struct SavedState {
    StateIdentity identity;
    QByteArray payload;
};

namespace StateFile {
RETROMEDIALAB_EMULATION_API QString hash(QByteArrayView data);
RETROMEDIALAB_EMULATION_API std::optional<SavedState> read(const QString& path);
RETROMEDIALAB_EMULATION_API bool write(const QString& path, const StateIdentity& identity, QByteArrayView payload);
RETROMEDIALAB_EMULATION_API bool isCompatible(const StateIdentity& state, const StateIdentity& expected);
}
}
