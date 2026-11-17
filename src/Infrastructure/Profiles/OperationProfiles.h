#pragma once
#include "InfrastructureApi.h"
#include "ProfileConstants.h"
#include <QJsonArray>
#include <QJsonValue>
#include <QMap>
#include <QSet>
#include <QString>
#include <array>
#include <functional>
#include <memory>
#include <vector>
namespace RetroMediaLab::Infrastructure {
enum class ProfileOperation {Read,Write,Convert};
struct OperationProfile {
    QString id,name;
    ProfileOperation operation=ProfileOperation::Read;
    QMap<QString,QString> values;
    QSet<QString> enabled;
    bool system=false;
};
class RETROMEDIALAB_INFRASTRUCTURE_API OperationProfiles final {
public:
    using Subscription=std::shared_ptr<const std::function<void()>>;
    OperationProfiles();
    const std::vector<OperationProfile>& forOperation(ProfileOperation)const;
    QString save(OperationProfile,bool replace=false);
    void rename(ProfileOperation,const QString& id,QString name);
    void remove(ProfileOperation,const QString& id);
    void reset(const QJsonValue&);
    QJsonArray toJson()const;
    Subscription subscribe(std::function<void()>);
private:
    void publish();
    static void sort(std::vector<OperationProfile>&);
    std::array<std::vector<OperationProfile>,ProfileConstants::OperationCount> profiles_;
    std::vector<std::weak_ptr<const std::function<void()>>> observers_;
};
}
