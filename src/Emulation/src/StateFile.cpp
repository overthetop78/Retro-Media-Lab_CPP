#include <RetroMediaLab/Emulation/StateFile.h>
#include <RetroMediaLab/Emulation/Messages.h>
#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QSaveFile>
#include <QThread>
#include <QtEndian>
#include <array>
#include <algorithm>
#include <limits>

namespace RetroMediaLab::Emulation::StateFile {
namespace {
constexpr QByteArrayView Magic("RetroMediaLabSTATE1");
constexpr qint32 Version = 1;
constexpr qint32 MaximumHeaderSize = 1024 * 1024;
constexpr int ReadAttempts = 64;
constexpr unsigned long ReadRetryDelayMilliseconds = 1;
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

void appendJsonString(QByteArray& output, QStringView value) {
    constexpr char Hex[] = "0123456789ABCDEF";
    constexpr auto HexMask = 0xF;
    constexpr auto FirstPrintableAscii = 0x20;
    constexpr auto LastPrintableAscii = 0x7E;
    constexpr QStringView EscapedAscii = u"\"'&+<>`";
    output.append('"');
    for (const QChar character : value) {
        switch (character.unicode()) {
        case u'\\': output.append("\\\\"); break;
        case u'\b': output.append("\\b"); break;
        case u'\f': output.append("\\f"); break;
        case u'\n': output.append("\\n"); break;
        case u'\r': output.append("\\r"); break;
        case u'\t': output.append("\\t"); break;
        default:
            const auto code = character.unicode();
            if (code >= FirstPrintableAscii && code <= LastPrintableAscii && !EscapedAscii.contains(character)) {
                output.append(character.toLatin1());
                break;
            }
            output.append("\\u");
            for (int shift : {12, 8, 4, 0}) output.append(Hex[(code >> shift) & HexMask]);
            break;
        }
    }
    output.append('"');
}

void appendCanonicalJson(QByteArray& output, const QJsonValue& value) {
    switch (value.type()) {
    case QJsonValue::Object: {
        const auto object = value.toObject();
        const auto keys = object.keys(); // QJsonObject returns its keys in ordinal order.
        output.append('{');
        bool first = true;
        for (const auto& key : keys) {
            if (!first) output.append(',');
            first = false;
            appendJsonString(output, key);
            output.append(':');
            appendCanonicalJson(output, object.value(key));
        }
        output.append('}');
        break;
    }
    case QJsonValue::Array: {
        const auto array = value.toArray();
        output.append('[');
        bool first = true;
        for (const auto& item : array) {
            if (!first) output.append(',');
            first = false;
            appendCanonicalJson(output, item);
        }
        output.append(']');
        break;
    }
    case QJsonValue::String: appendJsonString(output, value.toString()); break;
    case QJsonValue::Double: {
        constexpr auto MinInteger = std::numeric_limits<qint64>::min();
        constexpr auto MaxInteger = std::numeric_limits<qint64>::max();
        const auto integer = value.toInteger(MinInteger);
        if (integer != MinInteger || value.toInteger(MaxInteger) == MinInteger)
            output.append(QByteArray::number(integer));
        else
            output.append(QString::number(value.toDouble(), 'G', QLocale::FloatingPointShortest).toLatin1());
        break;
    }
    case QJsonValue::Bool: output.append(value.toBool() ? "true" : "false"); break;
    case QJsonValue::Null:
    case QJsonValue::Undefined: output.append("null"); break;
    }
}
}

QString hash(QByteArrayView data) {
    return QString::fromLatin1(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex().toUpper());
}

QString hashDocument(const QJsonValue& document) {
    QByteArray canonical;
    appendCanonicalJson(canonical, document);
    return hash(canonical);
}

std::optional<QString> hashStream(QIODevice& stream) {
    QCryptographicHash digest(QCryptographicHash::Sha256);
    if (!digest.addData(&stream)) return {};
    return QString::fromLatin1(digest.result().toHex().toUpper());
}

std::optional<QString> hashPath(const QString& path, const QString& pattern, QChar separator, qsizetype bufferSize) {
    if (path.isEmpty() || pattern.isEmpty() || bufferSize <= 0) return {};
    const QFileInfo content(path);
    if (content.isFile()) {
        QFile file(content.absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly)) return {};
        return hashStream(file);
    }
    if (!content.isDir() || !content.isReadable()) return {};

    const QDir root(content.absoluteFilePath());
    QStringList files;
    QDirIterator iterator(root.absolutePath(), {pattern}, QDir::Files | QDir::Hidden | QDir::System,
        QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        const auto filePath = iterator.next();
        if (iterator.fileInfo().isFile()) files.append(root.relativeFilePath(filePath));
    }
    std::ranges::sort(files, [](const QString& left, const QString& right) {
        const auto comparison = left.compare(right, Qt::CaseInsensitive);
        return comparison == 0 ? left < right : comparison < 0;
    });

    QCryptographicHash digest(QCryptographicHash::Sha256);
    QByteArray buffer(bufferSize, Qt::Uninitialized);
    for (const auto& relativePath : files) {
        auto name = relativePath;
        const auto utf8 = name.replace(u'/', separator).toUtf8();
        if (utf8.size() > std::numeric_limits<qint32>::max()) return {};
        std::array<char, sizeof(qint32)> length{};
        qToLittleEndian(static_cast<qint32>(utf8.size()), length.data());
        digest.addData(QByteArrayView(length.data(), length.size()));
        digest.addData(utf8);

        QFile file(root.filePath(relativePath));
        if (!file.open(QIODevice::ReadOnly)) return {};
        for (;;) {
            const auto read = file.read(buffer.data(), buffer.size());
            if (read < 0) return {};
            if (read == 0) break;
            digest.addData(QByteArrayView(buffer.constData(), read));
        }
    }
    return QString::fromLatin1(digest.result().toHex().toUpper());
}

namespace {
std::optional<SavedState> readOpened(QFile& file) {
    if (file.read(Magic.size()) != Magic) return std::nullopt;
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

std::optional<SavedState> readWithRetry(QFile& file) {
    for (int attempt = 1; ; ++attempt) {
        if (file.open(QIODevice::ReadOnly)) {
            auto state = readOpened(file);
            if (state || file.error() == QFileDevice::NoError) return state;
        }
        if (attempt == ReadAttempts) return {};
        file.close();
        QThread::msleep(ReadRetryDelayMilliseconds);
    }
}

EmulationMessageException stateError(MessageCode code, std::exception_ptr cause = {}) {
    return EmulationMessageException({MessageCategory::SavedState, code,
        MessageSeverity::Error, MessageTarget::Dialog, {}, {}}, std::move(cause));
}

EmulationMessageException stateIoError(const QFile& file) {
    return stateError(MessageCode::SavedStateOperationFailed,
        std::make_exception_ptr(std::runtime_error(file.errorString().toStdString())));
}
}

std::optional<SavedState> read(const QString& path) {
    QFile file(path);
    return readWithRetry(file);
}

SavedState readCompatible(const QString& path, const StateIdentity& expected) {
    QFile file(path);
    auto state = readWithRetry(file);
    if (!state) {
        if (file.error() != QFileDevice::NoError) throw stateIoError(file);
        throw stateError(MessageCode::SavedStateInvalid);
    }
    if (!isCompatible(state->identity, expected)) throw stateError(MessageCode::SavedStateIncompatible);
    return std::move(*state);
}

bool write(const QString& path, const StateIdentity& identity, QByteArrayView payload) {
    if (path.isEmpty() || !valid(identity) || payload.isEmpty() || payload.size() > MaximumPayloadSize) return false;
    const QJsonObject header{{FormatVersion, Version}, {Core, identity.core}, {CoreName, identity.coreName},
        {CoreVersion, identity.coreVersion}, {CoreHash, identity.coreSha256}, {Model, identity.model},
        {ConfigurationHash, identity.configurationSha256}, {ContentHash, identity.contentSha256}, {PayloadHash, hash(payload)}};
    const auto json = QJsonDocument(header).toJson(QJsonDocument::Compact);
    if (json.size() > MaximumHeaderSize) return false;
    std::array<char, sizeof(qint32)> length{};
    qToLittleEndian(static_cast<qint32>(json.size()), length.data());
    const QFileInfo target(path);
    if (!QDir().mkpath(target.absolutePath())) return false;
    QSaveFile file(target.absoluteFilePath());
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
