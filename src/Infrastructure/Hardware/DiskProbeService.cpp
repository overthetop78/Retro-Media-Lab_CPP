#include "DiskProbeService.h"
#include <HardwareDiscoveryConstants.h>
#include <StorageProbeConstants.h>
#include <stdexcept>
#include <limits>

namespace RetroMediaLab::Infrastructure {
using namespace StorageProbeConstants;
namespace {
[[noreturn]] void invalidReport(){throw std::runtime_error(HardwareDiscoveryConstants::DiskQueryFailed);}
int stateValue(const QJsonObject& object,const char* key,int maximum){
    const auto value=object.value(key);
    if(!value.isDouble()||value.toDouble()!=value.toInt(-1)||value.toInt(-1)<0||value.toInt()>maximum)invalidReport();
    return value.toInt();
}
}
DiskAccessReport inspectDiskInterface(const QString& controller,const QString& disk){
    for(const auto& selected:discoverHardware(HardwareDiscoveryConstants::Disks)){
        if(selected.id!=disk||(selected.parentId!=controller&&selected.id!=controller))continue;
        auto result=queryDiskAccess(selected.address,true);
        result.diskName=selected.name;
        result.controllerName=selected.parentName;result.controllerLocation=selected.parentLocation;result.controllerDriver=selected.parentDriver;
        result.commands=queryStorageCommands(selected);
        return result;
    }
    invalidReport();
}
DiskAccessReport runDiskProbe(const QString& controller,const QString& disk){
    auto result=inspectDiskInterface(controller,disk);
#ifdef _WIN32
    if(result.readState==DiskReadState::AccessDenied||result.commands.inquiryState==StorageCommandState::AccessDenied||result.commands.ataIdentifyState==StorageCommandState::AccessDenied)
        return requestElevatedDiskProbe(controller,disk,std::move(result));
#endif
    return result;
}
QJsonObject diskProbeReportToJson(const DiskAccessReport& r){
    return {{Connection,r.connection},{DiskName,r.diskName},{ControllerName,r.controllerName},{ControllerLocation,r.controllerLocation},{ControllerDriver,r.controllerDriver},
        {Pages,double(r.maximumPhysicalPages)},{BusMajor,int(r.busMajorVersion)},{BusMinor,int(r.busMinorVersion)},{Pio,r.adapterUsesPio},{Accelerated,r.acceleratedTransfer},
        {AddressKnown,r.addressKnown},{Port,int(r.portNumber)},{Path,int(r.pathId)},{Target,int(r.targetId)},{Lun,int(r.lun)},{Capacity,QString::number(r.capacityBytes)},
        {Logical,double(r.logicalSectorBytes)},{Physical,double(r.physicalSectorBytes)},
        {Transfer,double(r.maximumTransferBytes)},{Alignment,double(r.alignmentMask)},
        {Adapter,r.adapterPropertiesKnown},{Queue,r.commandQueueing},{ReadState,int(r.readState)},
        {Samples,r.sampledSectors},{Raw,r.rawAccessVerified},{NativeError,double(r.nativeError)},
        {FailedSector,QString::number(r.failedSector)},{Inquiry,int(r.commands.inquiryState)},
        {Ata,int(r.commands.ataIdentifyState)},{CommandError,double(r.commands.nativeError)},{InquiryError,double(r.commands.inquiryError)},{AtaError,double(r.commands.ataError)},{InquiryStatus,int(r.commands.inquiryStatus)},{AtaStatus,int(r.commands.ataStatus)},
        {Scsi,int(r.commands.scsiStatus)},{Transport,r.commands.transport},{Model,r.commands.identifiedModel}};
}
DiskAccessReport diskProbeReportFromJson(const QJsonObject& o){
    // The child returns metadata only. Reject truncated or inconsistent results.
    for(const auto* key:{Connection,DiskName,Capacity,FailedSector,Transport,Model,ControllerName,ControllerLocation,ControllerDriver})if(!o.value(key).isString())invalidReport();
    for(const auto* key:{Adapter,Queue,Raw,Pio,Accelerated,AddressKnown})if(!o.value(key).isBool())invalidReport();
    for(const auto* key:{Logical,Physical,Transfer,Alignment,NativeError,CommandError,Scsi,Pages,InquiryError,AtaError})if(!o.value(key).isDouble()||o.value(key).toDouble()<0||o.value(key).toDouble()>std::numeric_limits<quint32>::max()||o.value(key).toDouble()!=static_cast<quint32>(o.value(key).toDouble()))invalidReport();
    DiskAccessReport r;
    r.controllerName=o.value(ControllerName).toString();r.controllerLocation=o.value(ControllerLocation).toString();r.controllerDriver=o.value(ControllerDriver).toString();
    r.maximumPhysicalPages=static_cast<quint32>(o.value(Pages).toDouble());
    r.busMajorVersion=static_cast<quint16>(stateValue(o,BusMajor,std::numeric_limits<quint16>::max()));r.busMinorVersion=static_cast<quint16>(stateValue(o,BusMinor,std::numeric_limits<quint16>::max()));
    r.adapterUsesPio=o.value(Pio).toBool();r.acceleratedTransfer=o.value(Accelerated).toBool();r.addressKnown=o.value(AddressKnown).toBool();
    r.portNumber=static_cast<quint8>(stateValue(o,Port,HardwareDiscoveryConstants::MaximumByte));r.pathId=static_cast<quint8>(stateValue(o,Path,HardwareDiscoveryConstants::MaximumByte));
    r.targetId=static_cast<quint8>(stateValue(o,Target,HardwareDiscoveryConstants::MaximumByte));r.lun=static_cast<quint8>(stateValue(o,Lun,HardwareDiscoveryConstants::MaximumByte));
    r.connection=o.value(Connection).toString();r.diskName=o.value(DiskName).toString();
    bool valid=false;r.capacityBytes=o.value(Capacity).toString().toULongLong(&valid);if(!valid)invalidReport();
    r.failedSector=o.value(FailedSector).toString().toULongLong(&valid);if(!valid)invalidReport();
    r.logicalSectorBytes=static_cast<quint32>(o.value(Logical).toDouble());
    r.physicalSectorBytes=static_cast<quint32>(o.value(Physical).toDouble());
    r.maximumTransferBytes=static_cast<quint32>(o.value(Transfer).toDouble());r.alignmentMask=static_cast<quint32>(o.value(Alignment).toDouble());
    r.adapterPropertiesKnown=o.value(Adapter).toBool();r.commandQueueing=o.value(Queue).toBool();
    r.readState=static_cast<DiskReadState>(stateValue(o,ReadState,int(DiskReadState::Failed)));
    r.sampledSectors=stateValue(o,Samples,1);r.rawAccessVerified=o.value(Raw).toBool();
    if(r.rawAccessVerified!=(r.sampledSectors==1)||r.rawAccessVerified!=(r.readState==DiskReadState::Passed))invalidReport();
    r.nativeError=static_cast<quint32>(o.value(NativeError).toDouble());
    r.commands.inquiryState=static_cast<StorageCommandState>(stateValue(o,Inquiry,int(StorageCommandState::Failed)));
    r.commands.ataIdentifyState=static_cast<StorageCommandState>(stateValue(o,Ata,int(StorageCommandState::Failed)));
    r.commands.inquiryVerified=r.commands.inquiryState==StorageCommandState::Verified;
    r.commands.ataIdentifyVerified=r.commands.ataIdentifyState==StorageCommandState::Verified;
    r.commands.inquiryError=static_cast<quint32>(o.value(InquiryError).toDouble());r.commands.ataError=static_cast<quint32>(o.value(AtaError).toDouble());
    r.commands.inquiryStatus=static_cast<quint8>(stateValue(o,InquiryStatus,HardwareDiscoveryConstants::MaximumByte));r.commands.ataStatus=static_cast<quint8>(stateValue(o,AtaStatus,HardwareDiscoveryConstants::MaximumByte));
    r.commands.nativeError=static_cast<quint32>(o.value(CommandError).toDouble());
    r.commands.scsiStatus=static_cast<quint8>(stateValue(o,Scsi,HardwareDiscoveryConstants::MaximumByte));
    r.commands.transport=o.value(Transport).toString();r.commands.identifiedModel=o.value(Model).toString();
    return r;
}
}
