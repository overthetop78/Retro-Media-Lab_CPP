#pragma once
#include "InfrastructureApi.h"
#include <QDateTime>
#include <QString>
namespace RetroMediaLab {
enum class DiagnosticSeverity { Error, Warning, Information };
RETROMEDIALAB_INFRASTRUCTURE_API bool writeDiagnostic(const QString& directory,DiagnosticSeverity,const QString& detail,
    const QString& context,const QString& language,const QString& process,
    const QDateTime& time=QDateTime::currentDateTime()) noexcept;
}
