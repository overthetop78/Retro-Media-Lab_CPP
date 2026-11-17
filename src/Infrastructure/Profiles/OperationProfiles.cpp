#include "OperationProfiles.h"
#include <ErrorKeys.h>
#include <QJsonObject>
#include <QUuid>
#include <algorithm>
#include <stdexcept>
namespace RetroMediaLab::Infrastructure {
namespace C=ProfileConstants;
namespace {
std::size_t index(ProfileOperation operation){const auto result=static_cast<std::size_t>(operation);if(result>=C::OperationCount)throw std::invalid_argument(ErrorKeys::InvalidPreferencesFile);return result;}
void validateName(QString& name){name=name.trimmed();if(name.isEmpty() || name.contains(QChar::Null))throw std::invalid_argument(C::NameRequired);}
auto findProfile(std::vector<OperationProfile>& profiles,const QString& id){const auto found=std::find_if(profiles.begin(),profiles.end(),[&](const auto& profile){return profile.id==id;});if(found==profiles.end())throw std::invalid_argument(ErrorKeys::InvalidPreferencesFile);return found;}
bool sameName(const QString& left,const QString& right){return left.compare(right,Qt::CaseInsensitive)==0;}
}
OperationProfiles::OperationProfiles(){for(std::size_t i=0;i<C::OperationCount;++i)profiles_[i].push_back({QString::fromLatin1(C::DefaultPrefix)+QString::fromLatin1(C::Operations[i]).toLower(),C::DefaultName,static_cast<ProfileOperation>(i),{},{},true});}
const std::vector<OperationProfile>& OperationProfiles::forOperation(ProfileOperation operation)const{return profiles_[index(operation)];}
void OperationProfiles::sort(std::vector<OperationProfile>& values){std::sort(values.begin()+1,values.end(),[](const auto& a,const auto& b){return QString::localeAwareCompare(a.name,b.name)<0;});}
QString OperationProfiles::save(OperationProfile profile,bool replace){
    validateName(profile.name);if(profile.system)throw std::invalid_argument(C::SystemRename);
    auto& profiles=profiles_[index(profile.operation)];
    const auto existing=std::find_if(profiles.begin(),profiles.end(),[&](const auto& item){return sameName(item.name,profile.name);});
    if(existing!=profiles.end()){
        if(existing->system)throw std::invalid_argument(C::SystemRename);
        if(!replace)throw std::invalid_argument(C::Duplicate);
        profile.id=existing->id;*existing=profile;
    }else{profile.id=QUuid::createUuid().toString(QUuid::WithoutBraces);profiles.push_back(profile);}
    sort(profiles);publish();return profile.id;
}
void OperationProfiles::rename(ProfileOperation operation,const QString& id,QString name){
    validateName(name);auto& profiles=profiles_[index(operation)];auto found=findProfile(profiles,id);
    if(found->system)throw std::invalid_argument(C::SystemRename);
    if(std::any_of(profiles.begin(),profiles.end(),[&](const auto& item){return item.id!=id && sameName(item.name,name);}))throw std::invalid_argument(C::Duplicate);
    found->name=std::move(name);sort(profiles);publish();
}
void OperationProfiles::remove(ProfileOperation operation,const QString& id){auto& profiles=profiles_[index(operation)];const auto found=findProfile(profiles,id);if(found->system)throw std::invalid_argument(C::SystemDelete);profiles.erase(found);publish();}
QJsonArray OperationProfiles::toJson()const {
    QJsonArray result;
    for(const auto& profiles:profiles_)for(const auto& profile:profiles)if(!profile.system){
        QJsonObject values;for(auto it=profile.values.begin();it!=profile.values.end();++it)values.insert(it.key(),it.value());
        QJsonArray enabled;auto keys=profile.enabled.values();std::sort(keys.begin(),keys.end());for(const auto& key:keys)enabled.append(key);
        result.append(QJsonObject{{C::Id,profile.id},{C::Operation,C::Operations[index(profile.operation)]},{C::Name,profile.name},{C::Values,values},{C::Enabled,enabled}});
    }return result;
}
void OperationProfiles::reset(const QJsonValue& value){
    OperationProfiles parsed;QSet<QString> identifiers;
    for(const auto& profiles:parsed.profiles_)identifiers.insert(profiles.front().id);
    if(!value.isUndefined()){
        if(!value.isArray())throw std::invalid_argument(ErrorKeys::InvalidPreferencesFile);
        for(const auto& item:value.toArray()){
            if(!item.isObject())throw std::invalid_argument(ErrorKeys::InvalidPreferencesFile);const auto object=item.toObject();
            for(const auto* key:{C::Id,C::Operation,C::Name})if(!object.value(key).isString())throw std::invalid_argument(ErrorKeys::InvalidPreferencesFile);
            const auto operation=object.value(C::Operation).toString();const auto found=std::find(C::Operations.begin(),C::Operations.end(),operation);
            if(found==C::Operations.end() || !object.value(C::Values).isObject() || !object.value(C::Enabled).isArray())throw std::invalid_argument(ErrorKeys::InvalidPreferencesFile);
            OperationProfile profile;profile.id=object.value(C::Id).toString();profile.operation=static_cast<ProfileOperation>(found-C::Operations.begin());profile.name=object.value(C::Name).toString();validateName(profile.name);
            if(profile.id.isEmpty() || profile.id.contains(QChar::Null) || identifiers.contains(profile.id))throw std::invalid_argument(ErrorKeys::InvalidPreferencesFile);
            const auto values=object.value(C::Values).toObject();
            for(auto it=values.begin();it!=values.end();++it){if(it.key().isEmpty() || it.key().contains(QChar::Null) || !it.value().isString() || it.value().toString().contains(QChar::Null))throw std::invalid_argument(ErrorKeys::InvalidPreferencesFile);profile.values.insert(it.key(),it.value().toString());}
            for(const auto& key:object.value(C::Enabled).toArray()){if(!key.isString() || key.toString().isEmpty() || key.toString().contains(QChar::Null))throw std::invalid_argument(ErrorKeys::InvalidPreferencesFile);profile.enabled.insert(key.toString());}
            auto& profiles=parsed.profiles_[index(profile.operation)];if(std::any_of(profiles.begin(),profiles.end(),[&](const auto& item){return sameName(item.name,profile.name);}))throw std::invalid_argument(C::Duplicate);
            identifiers.insert(profile.id);profiles.push_back(std::move(profile));
        }
    }
    for(auto& profiles:parsed.profiles_)sort(profiles);profiles_=std::move(parsed.profiles_);publish();
}
OperationProfiles::Subscription OperationProfiles::subscribe(std::function<void()> observer){auto result=std::make_shared<const std::function<void()>>(std::move(observer));observers_.push_back(result);return result;}
void OperationProfiles::publish(){const auto observers=observers_;std::erase_if(observers_,[](const auto& item){return item.expired();});for(const auto& observer:observers)if(const auto callback=observer.lock())(*callback)();}
}
