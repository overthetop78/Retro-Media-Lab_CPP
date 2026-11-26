#include <RetroMediaLab/Emulation/StateFile.h>
#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QtEndian>
#include <array>
#include <limits>

namespace RetroMediaLab::Emulation::StateFile {
namespace {
constexpr QByteArrayView Magic("RetroMediaLabSTATE1");
constexpr qint32 Version = 1;
constexpr qint32 MaximumHeaderSize = 1024 * 1024;
constexpr qint64 MaximumPayloadSize = std::numeric_limits<qint32>::max() - 56;
constexpr QLatin1StringView FormatVersion("formatVersion");
constexpr QLatin1StringView Core("core");
constexpr QLatin1StringView CoreName("coreName");
constexpr QLatin1StringView CoreVersion("coreVersion");
constexpr QLatin1StringView CoreHash("coreSha256");
constexpr QLatin1StringView Model("model");
constexpr QLatin1StringView ConfigurationHash("configurationSha256");
constexpr QLatin1StringView ContentHash("contentSha256");
constexpr QLatin1StringView PayloadHash("stateSha256");

bool validIdentifier(const QJsonValue& value) {
    if (value.isString()) return !value.toString().trimmed().isEmpty();
    constexpr qint64 InvalidIdentifier = std::numeric_limits<qint64>::min();
    return value.isDouble() && value.toInteger(InvalidIdentifier) != InvalidIdentifier;
}

bool valid(const StateIdentity& identity) {
    return validIdentifier(identity.core) && validIdentifier(identity.model) &&
        !identity.coreName.trimmed().isEmpty() && !identity.coreVersion.trimmed().isEmpty() &&
        !identity.coreSha256.trimmed().isEmpty() && !identity.configurationSha256.trimmed().isEmpty() &&
        !identity.contentSha256.trimmed().isEmpty();
}

bool sameHash(const QString& a, const QString& b) {
    return a.compare(b, Qt::CaseInsensitive) == 0;
}
}

QString hash(QByteArrayView data) {
    return QString::fromLatin1(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex().toUpper());
}

std::optional<SavedState> read(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly) || file.read(Magic.size()) != Magic) return std::nullopt;
    const auto sizeBytes = file.read(sizeof(qint32));
    if (sizeBytes.size() != sizeof(qint32)) return std::nullopt;
    const auto headerSize = qFromLittleEndian<qint32>(sizeBytes.constData());
    if (headerSize <= 0 || headerSize > MaximumHeaderSize) return std::nullopt;
    const auto payloadSize = file.size() - file.pos() - headerSize;
    if (payloadSize <= 0 || payloadSize > MaximumPayloadSize) return std::nullopt;
    const auto headerBytes = file.read(headerSize);
    if (headerBytes.size() != headerSize) return std::nullopt;
    const auto header = QJsonDocument::fromJson(headerBytes).object();
    StateIdentity identity{header.value(Core), header.value(CoreName).toString(),
        header.value(CoreVersion).toString(), header.value(CoreHash).toString(),
        header.value(Model), header.value(ConfigurationHash).toString(), header.value(ContentHash).toString()};
    if (header.value(FormatVersion).toInt() != Version || !valid(identity)) return std::nullopt;
    auto payload = file.read(payloadSize);
    if (payload.size() != payloadSize || !file.atEnd() ||
        !sameHash(header.value(PayloadHash).toString(), hash(payload))) return std::nullopt;
    return SavedState{std::move(identity), std::move(payload)};
}

bool write(const QString& path, const StateIdentity& identity, QByteArrayView payload) {
    if (!valid(identity) || payload.isEmpty() || payload.size() > MaximumPayloadSize) return false;
    const QJsonObject header{{FormatVersion, Version}, {Core, identity.core}, {CoreName, identity.coreName},
        {CoreVersion, identity.coreVersion}, {CoreHash, identity.coreSha256}, {Model, identity.model},
        {ConfigurationHash, identity.configurationSha256}, {ContentHash, identity.contentSha256}, {PayloadHash, hash(payload)}};
    const auto json = QJsonDocument(header).toJson(QJsonDocument::Compact);
    if (json.size() > MaximumHeaderSize) return false;
    std::array<char, sizeof(qint32)> length{};
    qToLittleEndian(static_cast<qint32>(json.size()), length.data());
    QSaveFile file(path);
    return file.open(QIODevice::WriteOnly) &&
        file.write(Magic.data(), Magic.size()) == Magic.size() && file.write(length.data(), static_cast<qint64>(length.size())) == static_cast<qint64>(length.size()) &&
        file.write(json) == json.size() && file.write(payload.data(), payload.size()) == payload.size() && file.commit();
}

bool isCompatible(const StateIdentity& state, const StateIdentity& expected) {
    return valid(state) && valid(expected) && state.core == expected.core && state.model == expected.model &&
        state.coreName == expected.coreName && state.coreVersion == expected.coreVersion &&
        sameHash(state.coreSha256, expected.coreSha256) && sameHash(state.contentSha256, expected.contentSha256) &&
        sameHash(state.configurationSha256, expected.configurationSha256);
}
}
