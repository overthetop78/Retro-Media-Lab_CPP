#include "HardwareSettings.h"
#include <ErrorKeys.h>
#include <QJsonArray>
#include <QMap>
#include <QUuid>
#include <QFileInfo>
#include <algorithm>
#include <stdexcept>
namespace RetroMediaLab {
using namespace HardwareConstants;
const QVector<HardwareCategory>& hardwareCategories() {
    static const QVector<HardwareCategory> categories{{Floppy,FloppyKey},{Optical,OpticalKey},{Audio,AudioKey},
        {Disks,DisksKey},{Cartridge,CartridgeKey},{Tape,TapeKey}};
    return categories;
}
namespace {
bool valid(const HardwareDevice& device) {
    return std::any_of(hardwareCategories().cbegin(),hardwareCategories().cend(),[&](const auto& category){return device.category==category.id;})
        && !device.name.trimmed().isEmpty() && !device.address.trimmed().isEmpty()
        && device.channels>0 && device.channels<=ChannelMax && device.channel>=0 && device.channel<device.channels
        && device.sampleRate>=RateMin && device.sampleRate<=RateMax;
}
void assignSelections(QVector<HardwareDevice>& devices) {
    QMap<QString,int> next;
    for(auto& device:devices)if(device.category==Floppy && device.backend==Greaseweazle)
        device.selection=next[device.backend+device.controller.toLower()]++==0?SelectA:SelectB;
}
}
bool HardwareSettings::saveDevice(HardwareDevice device) {
    if(!valid(device))return false;
    if(device.category==Floppy && (device.backend==Greaseweazle||device.backend==KryoFlux)) {
        if(device.controller.isEmpty())device.controller=device.address;
        const auto count=std::count_if(devices.cbegin(),devices.cend(),[&](const auto& other){
            return other.id!=device.id && other.category==Floppy && other.backend==device.backend
                && other.controller.compare(device.controller,Qt::CaseInsensitive)==0;
        });
        if(count>=MaxGwDrives)return false;
    }
    if(device.category==Floppy&&device.backend==KryoFlux){
        const auto occupied=[&](const QString& selection){return std::any_of(devices.cbegin(),devices.cend(),[&](const auto& other){return other.id!=device.id&&other.category==Floppy&&other.backend==KryoFlux&&other.controller.compare(device.controller,Qt::CaseInsensitive)==0&&other.selection==selection;});};
        if(device.selection.isEmpty())device.selection=occupied(Select0)?Select1:Select0;
        if((device.selection!=Select0&&device.selection!=Select1)||occupied(device.selection))return false;
    }
    if(device.id.isEmpty())device.id=QUuid::createUuid().toString(QUuid::WithoutBraces);
    auto found=std::find_if(devices.begin(),devices.end(),[&](const auto& other){return other.id==device.id;});
    if(found==devices.end())devices.push_back(std::move(device));else *found=std::move(device);
    assignSelections(devices);return true;
}
void HardwareSettings::forgetDevice(const QString& id) {
    devices.removeIf([&](const auto& device){return device.id==id;});assignSelections(devices);
}
void HardwareSettings::applyFloppyDiscovery(const QString& backend,const QVector<Infrastructure::HostDevice>& found){
    for(auto& device:devices)if(device.category==Floppy&&device.backend==backend){device.available=false;
        for(const auto& host:found)if(host.backend==backend&&(device.controller.compare(host.id,Qt::CaseInsensitive)==0||(backend==Greaseweazle&&device.address==host.address)||(backend==KryoFlux&&device.controller==DefaultKryoController&&found.size()==1))){device.available=host.available;device.address=host.address;device.controller=host.id;break;}}
}
void HardwareSettings::applyMediaDiscovery(const QString& category,const QVector<Infrastructure::HostDevice>& found,const QVector<Infrastructure::DiskController>& controllers){
    for(auto& device:devices)if(device.category==category){device.available=false;
        if(category==Disks)for(const auto& controller:controllers)if(controller.id==device.controller){device.available=true;break;}
        for(const auto& host:found)if(device.controller==host.id){device.available=host.available;device.address=host.address;break;}}
}
bool HardwareSettings::saveDiskInterface(const Infrastructure::DiskController& controller){
    if(controller.id.isEmpty()||std::any_of(devices.cbegin(),devices.cend(),[&](const auto& device){return device.category==Disks&&device.controller==controller.id;}))return false;
    HardwareDevice device;device.category=Disks;device.name=controller.name;device.controller=controller.id;device.address=controller.id;device.backend=controller.connection;device.available=true;return saveDevice(std::move(device));
}
bool HardwareSettings::saveDetectedDevice(const QString& category,const Infrastructure::HostDevice& host){
    if(category==Disks||std::any_of(devices.cbegin(),devices.cend(),[&](const auto& device){return device.category==category&&device.controller==host.id;}))return false;
    HardwareDevice device;device.category=category;device.name=host.name;device.address=host.address;device.controller=host.id;device.backend=host.backend;device.available=host.available;return saveDevice(std::move(device));
}
void HardwareSettings::selectGwPath(const QString& path){
    if(gwPath.compare(path,Qt::CaseInsensitive)!=0&&QFileInfo(gwPath).isFile())previousGwPath=gwPath;gwPath=path;
}
QJsonObject HardwareSettings::toJson() const {
    QJsonArray array;
    for(const auto& device:devices)array.append(QJsonObject{
        {Id,device.id},{Category,device.category},{Backend,device.backend},{Name,device.name},{Address,device.address},
        {Controller,device.controller},{Size,device.size},{Density,device.density},{Rpm,device.rpm},{Selection,device.selection},
        {Rate,device.sampleRate},{Channels,device.channels},{Channel,device.channel}});
    return {{GwPath,gwPath},{PreviousGwPath,previousGwPath},{DtcPath,dtcPath},{SelectedFloppyBackend,floppyBackend},{Devices,array}};
}
HardwareSettings HardwareSettings::fromJson(const QJsonValue& value) {
    if(value.isUndefined())return {};
    const auto fail=[](){throw std::runtime_error(ErrorKeys::InvalidPreferencesFile);};
    if(!value.isObject())fail();
    const auto object=value.toObject();
    if(!object.value(GwPath).isString()||!object.value(PreviousGwPath).isString()||!object.value(Devices).isArray())fail();
    HardwareSettings settings;settings.gwPath=object.value(GwPath).toString();settings.previousGwPath=object.value(PreviousGwPath).toString();
    for(const auto* key:{DtcPath,SelectedFloppyBackend})if(!object.value(key).isUndefined()&&!object.value(key).isString())fail();
    settings.dtcPath=object.value(DtcPath).toString();settings.floppyBackend=object.value(SelectedFloppyBackend).toString(Greaseweazle);
    if(settings.floppyBackend!=Greaseweazle&&settings.floppyBackend!=KryoFlux)fail();
    for(const auto& item:object.value(Devices).toArray()) {
        if(!item.isObject())fail();const auto data=item.toObject();HardwareDevice device;
        for(const auto* key:{Id,Category,Backend,Name,Address,Controller,Size,Density,Selection})if(!data.value(key).isString())fail();
        for(const auto* key:{Rate,Channels,Channel,Rpm})if(!data.value(key).isDouble()||data.value(key).toDouble()!=data.value(key).toInt())fail();
        device.id=data.value(Id).toString();device.category=data.value(Category).toString();device.backend=data.value(Backend).toString();
        device.name=data.value(Name).toString();device.address=data.value(Address).toString();device.controller=data.value(Controller).toString();
        device.size=data.value(Size).toString();device.density=data.value(Density).toString();device.rpm=data.value(Rpm).toInt();device.selection=data.value(Selection).toString();
        device.channels=data.value(Channels).toInt();device.channel=data.value(Channel).toInt();device.sampleRate=data.value(Rate).toInt();
        if(device.id.isEmpty()||std::any_of(settings.devices.cbegin(),settings.devices.cend(),[&](const auto& other){return other.id==device.id;})||!settings.saveDevice(device))fail();
    }
    return settings;
}
}
