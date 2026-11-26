#pragma once

#include "EmulationExport.h"
#include <QJsonDocument>
#include <QList>
#include <QStringList>
#include <QUuid>
#include <optional>
#include <stop_token>

namespace RetroMediaLab::Emulation::ConfigurationFiles {

inline constexpr auto MachineFileName = "machine.json";

struct Document {
    QString path;
    QJsonDocument json;
};

enum class WriteResult { Saved, Cancelled, InvalidDocument, IoFailure };

RETROMEDIALAB_EMULATION_API QString storePath(const QString& path, const QString& base);
RETROMEDIALAB_EMULATION_API QString resolvePath(const QString& storedPath, const QString& base);
RETROMEDIALAB_EMULATION_API QString machinePath(const QString& directory, const QUuid&);
RETROMEDIALAB_EMULATION_API QStringList enumerateDocumentPaths(const QString& directory,
    const QString& fileName = QLatin1String(MachineFileName));
RETROMEDIALAB_EMULATION_API std::optional<QJsonDocument> readDocument(const QString& path, std::stop_token = {});
// A disengaged result indicates cancellation. Unreadable and malformed entries are skipped.
RETROMEDIALAB_EMULATION_API std::optional<QList<Document>> readDocuments(const QString& directory,
    const QString& fileName = QLatin1String(MachineFileName), std::stop_token = {});
RETROMEDIALAB_EMULATION_API WriteResult writeDocument(const QString& path, const QJsonDocument&, std::stop_token = {});

}
