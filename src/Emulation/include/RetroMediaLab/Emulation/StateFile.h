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
// The input is a JSON value. Qt's representation does not retain source numeric types or spelling.
RETROMEDIALAB_EMULATION_API QString hashDocument(const QJsonValue& document);
// Reads from the current position. The caller owns the device.
RETROMEDIALAB_EMULATION_API std::optional<QString> hashStream(QIODevice& stream);
RETROMEDIALAB_EMULATION_API std::optional<QString> hashPath(const QString& path, const QString& pattern,
    QChar separator, qsizetype bufferSize);
RETROMEDIALAB_EMULATION_API std::optional<SavedState> read(const QString& path);
// Reports I/O, malformed state and identity mismatch through EmulationMessageException.
RETROMEDIALAB_EMULATION_API SavedState readCompatible(const QString& path, const StateIdentity& expected);
RETROMEDIALAB_EMULATION_API bool write(const QString& path, const StateIdentity& identity, QByteArrayView payload);
RETROMEDIALAB_EMULATION_API bool isCompatible(const StateIdentity& state, const StateIdentity& expected);
}
}
