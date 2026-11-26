#pragma once

#include "EmulationExport.h"
#include "Sdk.h"
#include <QLocale>

namespace RetroMediaLab::Emulation::HardwareSettings {

struct Description {
    QString fieldPrefix;
    QString cpuResourceKey;
    std::optional<QString> cpuFrequency, mainMemory, displayMemory, resolution;
    QString displayMemoryResourceKey;
};

RETROMEDIALAB_EMULATION_API void appendBlocks(QList<SettingsBlock>&, const Description&);
RETROMEDIALAB_EMULATION_API void appendBlocks(QList<SettingsBlock>&, const Description&, SettingsPresentation&, bool compactCpu);
RETROMEDIALAB_EMULATION_API SettingsChoice invariantChoice(QString id, QString text);
RETROMEDIALAB_EMULATION_API SettingsField information(QString id, QString label, QString value, QString help = {});
RETROMEDIALAB_EMULATION_API SettingsField number(QString id, QString label, QString value, QString help = {});
RETROMEDIALAB_EMULATION_API SettingsField toggle(QString id, QString label, bool value, QString help = {});
RETROMEDIALAB_EMULATION_API SettingsField select(QString id, QString label, QString value, QList<SettingsChoice>,
    QString help = {}, bool enabled = true);
RETROMEDIALAB_EMULATION_API SettingsField audioSelection(const QHash<QString, QString>& options,
    QString id, QString label, int fallback, const QList<int>& values, const QString& unit,
    const QLocale& = QLocale());
RETROMEDIALAB_EMULATION_API QString formatFrequency(qint64 hertz, bool useKilohertz = false,
    const QLocale& = QLocale());
RETROMEDIALAB_EMULATION_API QString formatMegahertz(qint64 hertz, int minimumDecimals, int maximumDecimals,
    const QLocale& = QLocale());
RETROMEDIALAB_EMULATION_API QString formatMemory(qint64 bytes);

}
