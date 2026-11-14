#pragma once
#include "InfrastructureApi.h"
#include <QObject>
#include <QByteArray>
#include <QPointer>
#include <QUrl>
#include <functional>
#include <memory>
class QNetworkAccessManager;
class QNetworkReply;
namespace RetroMediaLab {
struct HostToolsRelease { QString version; QUrl url; QString sha256; };
class HostToolsService final : public QObject {
public:
    RETROMEDIALAB_INFRASTRUCTURE_API explicit HostToolsService(QObject* = nullptr);
    RETROMEDIALAB_INFRASTRUCTURE_API ~HostToolsService() override;
    RETROMEDIALAB_INFRASTRUCTURE_API void checkLatest(std::function<void(HostToolsRelease)>,std::function<void(const char*)>);
    RETROMEDIALAB_INFRASTRUCTURE_API void download(const HostToolsRelease&,const QString& root,std::function<void(QString)>,
        std::function<void(int)>,std::function<void(const char*)>);
    RETROMEDIALAB_INFRASTRUCTURE_API static QString installArchive(const QByteArray&,const QString& root,const QString& version);
    RETROMEDIALAB_INFRASTRUCTURE_API static QString findExecutable(const QString& configured,const QString& managedRoot,const QString& applicationRoot);
    RETROMEDIALAB_INFRASTRUCTURE_API static QString findDtcExecutable(const QString& configured,const QString& managedRoot,const QString& applicationRoot);
    RETROMEDIALAB_INFRASTRUCTURE_API static bool validExecutable(const QString&);
    RETROMEDIALAB_INFRASTRUCTURE_API static QString managedDirectory(const QString& logsDirectory,const QString& toolFolder);
private:
    std::unique_ptr<QNetworkAccessManager> network_;
    QPointer<QNetworkReply> reply_;
};
}
