#pragma once

#include "EmulationExport.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QStringList>
#include <QUuid>
#include <functional>
#include <optional>
#include <stop_token>

namespace RetroMediaLab::Emulation::ConfigurationFiles {

inline constexpr auto MachineFileName = "machine.json";

struct Document {
    QString path;
    QJsonDocument json;
};

namespace MachineDocumentKeys {
inline constexpr QLatin1StringView SchemaVersion("schemaVersion");
inline constexpr QLatin1StringView Id("id");
inline constexpr QLatin1StringView Model("model");
inline constexpr QLatin1StringView Core("core");
inline constexpr QLatin1StringView Firmware("firmware");
inline constexpr QLatin1StringView Firmwares("firmwares");
inline constexpr QLatin1StringView Media("media");
inline constexpr QLatin1StringView Options("options");
inline constexpr QLatin1StringView Input("input");
inline constexpr QLatin1StringView Folders("folders");
inline constexpr QLatin1StringView AudioEnabled("audioEnabled");
inline constexpr QLatin1StringView ConfiguredSlots("configuredSlots");
inline constexpr QLatin1StringView InitialContent("initialContent");
}

struct MachineDocument {
    QUuid id;
    QJsonObject fields;

    RETROMEDIALAB_EMULATION_API QJsonValue value(QLatin1StringView name) const;
};

// The module interprets its model, core, firmware, and other option values.
RETROMEDIALAB_EMULATION_API std::optional<MachineDocument> decodeMachineDocument(const QJsonDocument&, int schemaVersion);

enum class WriteResult { Saved, Cancelled, InvalidDocument, IoFailure };
enum class RestoreResult { Complete, Cancelled, DirectoryError };
enum class MachineDirectoryRemoval { EmptyOnly, Recursive };
enum class RemoveResult { Removed, Missing, Cancelled, InvalidDirectory, IoFailure };

// A module validates and restores its own fields. It can omit an invalid
// configuration without interrupting the other documents in the directory.
using MachineDocumentRestorer = std::function<void(const MachineDocument&, const QString& path, std::stop_token)>;
RETROMEDIALAB_EMULATION_API RestoreResult restoreMachineDocuments(const QString& directory, int schemaVersion,
    const MachineDocumentRestorer&, std::stop_token = {});

RETROMEDIALAB_EMULATION_API QString storePath(const QString& path, const QString& base);
RETROMEDIALAB_EMULATION_API QString resolvePath(const QString& storedPath, const QString& base);
RETROMEDIALAB_EMULATION_API QString machinePath(const QString& directory, const QUuid&);
RETROMEDIALAB_EMULATION_API RemoveResult removeMachineDirectory(const QString& directory, const QUuid&,
    MachineDirectoryRemoval, std::stop_token = {});
RETROMEDIALAB_EMULATION_API QStringList enumerateDocumentPaths(const QString& directory,
    const QString& fileName = QLatin1String(MachineFileName));
RETROMEDIALAB_EMULATION_API std::optional<QJsonDocument> readDocument(const QString& path, std::stop_token = {});
// A disengaged result indicates cancellation. Unreadable and malformed entries are skipped.
RETROMEDIALAB_EMULATION_API std::optional<QList<Document>> readDocuments(const QString& directory,
    const QString& fileName = QLatin1String(MachineFileName), std::stop_token = {});
RETROMEDIALAB_EMULATION_API WriteResult writeDocument(const QString& path, const QJsonDocument&, std::stop_token = {});

}
