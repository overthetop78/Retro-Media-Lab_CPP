#include <HardwareDiscoveryConstants.h>
#include <RetroMediaLab/Infrastructure/HardwareDiscovery.h>
#include <windows.h>
#include <setupapi.h>
#include <devguid.h>
#include <ntddcdrm.h>
#include <ntddmmc.h>
#include <ntddscsi.h>
#include <winioctl.h>
#include <cfgmgr32.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <devpkey.h>
#include <cstring>
#include <algorithm>
#include <wrl/client.h>
#include <memory>
#include <stdexcept>
#include <vector>
namespace RetroMediaLab::Infrastructure {
using namespace HardwareDiscoveryConstants;
using Microsoft::WRL::ComPtr;
namespace {
struct DeviceListCloser { void operator()(void* handle) const noexcept { if(handle!=INVALID_HANDLE_VALUE)SetupDiDestroyDeviceInfoList(handle); } };
struct RegistryCloser { void operator()(HKEY handle) const noexcept { if(handle && handle!=INVALID_HANDLE_VALUE)RegCloseKey(handle); } };
struct TaskMemoryCloser { void operator()(wchar_t* memory) const noexcept { CoTaskMemFree(memory); } };
struct HandleCloser { void operator()(void* handle) const noexcept { if(handle&&handle!=INVALID_HANDLE_VALUE)CloseHandle(handle); } };
bool control(HANDLE device,DWORD code,void* input,DWORD inputSize,void* output,DWORD outputSize,DWORD& received){
    std::unique_ptr<void,HandleCloser> event(CreateEventW(nullptr,TRUE,FALSE,nullptr));if(!event)return false;
    OVERLAPPED operation{};operation.hEvent=event.get();
    if(DeviceIoControl(device,code,input,inputSize,output,outputSize,&received,&operation))return true;
    if(GetLastError()!=ERROR_IO_PENDING)return false;
    if(WaitForSingleObject(event.get(),ConfigurationTimeoutMs)!=WAIT_OBJECT_0){CancelIoEx(device,&operation);GetOverlappedResult(device,&operation,&received,TRUE);return false;}
    return GetOverlappedResult(device,&operation,&received,FALSE)!=FALSE;
}
QString busName(STORAGE_BUS_TYPE type){
    switch(type){case BusTypeUsb:return UsbConnection;case BusTypeSata:return SataConnection;case BusTypeAta:return AtaConnection;case BusTypeScsi:return ScsiConnection;case BusTypeSas:return SasConnection;case BusTypeNvme:return NvmeConnection;default:return {};}
}
QString nodeProperty(DEVINST node,ULONG key){
    ULONG size=0;CM_Get_DevNode_Registry_PropertyW(node,key,nullptr,nullptr,&size,0);if(!size)return {};
    std::vector<wchar_t> value(size/sizeof(wchar_t)+1);
    if(CM_Get_DevNode_Registry_PropertyW(node,key,nullptr,value.data(),&size,0)!=CR_SUCCESS)return {};
    return QString::fromWCharArray(value.data());
}
QString nodeId(DEVINST node){
    std::vector<wchar_t> value(BufferSize);if(CM_Get_Device_IDW(node,value.data(),static_cast<ULONG>(value.size()),0)!=CR_SUCCESS)return {};
    return QString::fromWCharArray(value.data());
}
DiskController controllerNode(DEVINST node){
    DiskController result;result.id=nodeId(node);result.name=nodeProperty(node,CM_DRP_FRIENDLYNAME);if(result.name.isEmpty())result.name=nodeProperty(node,CM_DRP_DEVICEDESC);
    if(result.id.startsWith(UsbVendorPrefix,Qt::CaseInsensitive)){
        DEVPROPTYPE type=0;ULONG size=0;CM_Get_DevNode_PropertyW(node,&DEVPKEY_Device_BusReportedDeviceDesc,&type,nullptr,&size,0);
        if(size){std::vector<BYTE> value(size);if(CM_Get_DevNode_PropertyW(node,&DEVPKEY_Device_BusReportedDeviceDesc,&type,value.data(),&size,0)==CR_SUCCESS&&type==DEVPROP_TYPE_STRING)result.name=QString::fromWCharArray(reinterpret_cast<const wchar_t*>(value.data()));}
    }
    result.location=nodeProperty(node,CM_DRP_LOCATION_INFORMATION);result.driver=nodeProperty(node,CM_DRP_SERVICE);
    if(result.id.startsWith(PciPrefix,Qt::CaseInsensitive))result.connection=PciConnection;
    else if(result.id.startsWith(UsbVendorPrefix,Qt::CaseInsensitive))result.connection=UsbConnection;
    return result;
}
DiskController parentController(DEVINST node){
    for(int depth=0;depth<ParentDepthLimit;++depth){
        DEVINST parent=0;if(CM_Get_Parent(&parent,node,0)!=CR_SUCCESS)break;node=parent;
        const auto id=nodeId(node);const auto className=nodeProperty(node,CM_DRP_CLASSGUID);GUID category{};
        const auto valid=SUCCEEDED(CLSIDFromString(reinterpret_cast<LPCOLESTR>(className.utf16()),&category));
        if(id.startsWith(UsbVendorPrefix,Qt::CaseInsensitive)||(valid&&(IsEqualGUID(category,GUID_DEVCLASS_HDC)||IsEqualGUID(category,GUID_DEVCLASS_SCSIADAPTER))))return controllerNode(node);
    }
    return {};
}
struct Apartment {
    HRESULT result=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
    ~Apartment(){if(SUCCEEDED(result))CoUninitialize();}
};
struct Property {
    PROPVARIANT value;
    Property(){PropVariantInit(&value);}
    ~Property(){PropVariantClear(&value);}
};
QString property(HDEVINFO list,SP_DEVINFO_DATA& device,DWORD key) {
    DWORD bytes=0;
    SetupDiGetDeviceRegistryPropertyW(list,&device,key,nullptr,nullptr,0,&bytes);
    if(!bytes)return {};
    std::vector<wchar_t> buffer(bytes/sizeof(wchar_t)+1);
    if(!SetupDiGetDeviceRegistryPropertyW(list,&device,key,nullptr,reinterpret_cast<PBYTE>(buffer.data()),bytes,nullptr))return {};
    return QString::fromWCharArray(buffer.data());
}
QVector<HostDevice> classDevices(const GUID& category,bool serial,bool kryoflux=false) {
    const GUID* interfaceId=IsEqualGUID(category,GUID_DEVCLASS_CDROM)?&GUID_DEVINTERFACE_CDROM:IsEqualGUID(category,GUID_DEVCLASS_DISKDRIVE)?&GUID_DEVINTERFACE_DISK:IsEqualGUID(category,GUID_DEVCLASS_TAPEDRIVE)?&GUID_DEVINTERFACE_TAPE:nullptr;
    const auto handle=SetupDiGetClassDevsW(kryoflux?nullptr:interfaceId?interfaceId:&category,nullptr,nullptr,DIGCF_PRESENT|(kryoflux?DIGCF_ALLCLASSES:interfaceId?DIGCF_DEVICEINTERFACE:0));
    if(handle==INVALID_HANDLE_VALUE)throw std::runtime_error(Failure);
    std::unique_ptr<void,DeviceListCloser> list(handle);
    QVector<HostDevice> result;
    for(DWORD index=0;;++index) {
        SP_DEVINFO_DATA device{};device.cbSize=sizeof(device);
        if(!SetupDiEnumDeviceInfo(handle,index,&device)) {
            if(GetLastError()!=ERROR_NO_MORE_ITEMS)throw std::runtime_error(Failure);
            break;
        }
        std::vector<wchar_t> instance(BufferSize);
        if(!SetupDiGetDeviceInstanceIdW(handle,&device,instance.data(),static_cast<DWORD>(instance.size()),nullptr))continue;
        const auto id=QString::fromWCharArray(instance.data());
        if(kryoflux&&!id.startsWith(UsbIdPrefix,Qt::CaseInsensitive))continue;
        auto name=property(handle,device,SPDRP_FRIENDLYNAME);
        if(name.isEmpty())name=property(handle,device,SPDRP_DEVICEDESC);
        QString address;
        if(kryoflux){
            if(!name.contains(KryoFlux,Qt::CaseInsensitive)&&!property(handle,device,SPDRP_SERVICE).contains(KryoFlux,Qt::CaseInsensitive))continue;
            address=UsbAddress;
        } else if(serial) {
            const auto ids=property(handle,device,SPDRP_HARDWAREID);
            const bool candidate=(ids.contains(Vid,Qt::CaseInsensitive)&&(ids.contains(Pid,Qt::CaseInsensitive)||ids.contains(CompatiblePid,Qt::CaseInsensitive)))
                ||name.contains(Greaseweazle,Qt::CaseInsensitive)||name.contains(Compatible,Qt::CaseInsensitive)
                ||id.section(QChar::fromLatin1(Backslash),LastSection).startsWith(SerialPrefix,Qt::CaseInsensitive);
            if(!candidate)continue;
            const auto key=SetupDiOpenDevRegKey(handle,&device,DICS_FLAG_GLOBAL,0,DIREG_DEV,KEY_READ);
            if(key==INVALID_HANDLE_VALUE)continue;
            std::unique_ptr<std::remove_pointer_t<HKEY>,RegistryCloser> registry(key);
            std::vector<wchar_t> port(BufferSize);DWORD size=static_cast<DWORD>(port.size()*sizeof(wchar_t)),type=0;
            if(RegQueryValueExW(key,PortName,nullptr,&type,reinterpret_cast<BYTE*>(port.data()),&size)!=ERROR_SUCCESS||type!=REG_SZ)continue;
            address=QString::fromWCharArray(port.data());
        } else {
            SP_DEVICE_INTERFACE_DATA interfaceData{};interfaceData.cbSize=sizeof(interfaceData);
            if(!interfaceId||!SetupDiEnumDeviceInterfaces(handle,&device,interfaceId,0,&interfaceData))continue;
            DWORD length=0;SetupDiGetDeviceInterfaceDetailW(handle,&interfaceData,nullptr,0,&length,nullptr);
            if(length<sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W))continue;
            std::vector<unsigned char> storage(length);auto* detail=reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA_W*>(storage.data());detail->cbSize=sizeof(*detail);
            if(!SetupDiGetDeviceInterfaceDetailW(handle,&interfaceData,detail,length,nullptr,nullptr))continue;
            address=QString::fromWCharArray(detail->DevicePath);
        }
        if(!address.isEmpty()){
            HostDevice host{id,name,address,kryoflux?KryoFlux:serial?Greaseweazle:Native};
            if(IsEqualGUID(category,GUID_DEVCLASS_DISKDRIVE)){
                const auto parent=parentController(device.DevInst);host.parentId=parent.id;host.parentName=parent.name;host.connection=parent.connection;host.parentLocation=parent.location;host.parentDriver=parent.driver;
            }
            result.push_back(std::move(host));
        }
    }
    return result;
}
QVector<HostDevice> audioDevices() {
    Apartment apartment;
    if(FAILED(apartment.result)&&apartment.result!=RPC_E_CHANGED_MODE)throw std::runtime_error(Failure);
    ComPtr<IMMDeviceEnumerator> enumerator;
    if(FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator),nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&enumerator))))throw std::runtime_error(Failure);
    ComPtr<IMMDeviceCollection> collection;
    if(FAILED(enumerator->EnumAudioEndpoints(eCapture,DEVICE_STATE_ACTIVE|DEVICE_STATE_DISABLED|DEVICE_STATE_UNPLUGGED,&collection)))throw std::runtime_error(Failure);
    UINT count=0;if(FAILED(collection->GetCount(&count)))throw std::runtime_error(Failure);
    QVector<HostDevice> result;
    for(UINT index=0;index<count;++index) {
        ComPtr<IMMDevice> device;if(FAILED(collection->Item(index,&device)))continue;
        wchar_t* raw=nullptr;const auto status=device->GetId(&raw);
        std::unique_ptr<wchar_t,TaskMemoryCloser> id(raw);
        if(FAILED(status))continue;
        ComPtr<IPropertyStore> properties;if(FAILED(device->OpenPropertyStore(STGM_READ,&properties)))continue;
        Property name;if(FAILED(properties->GetValue(PKEY_Device_FriendlyName,&name.value))||name.value.vt!=VT_LPWSTR)continue;
        DWORD state=0;if(FAILED(device->GetState(&state)))continue;
        HostDevice host{QString::fromWCharArray(id.get()),QString::fromWCharArray(name.value.pwszVal),QString::fromWCharArray(id.get()),Wasapi};
        host.available=(state&DEVICE_STATE_ACTIVE)!=0;
        host.state=state&DEVICE_STATE_DISABLED?HostDeviceState::Disabled:state&DEVICE_STATE_UNPLUGGED?HostDeviceState::Unplugged:HostDeviceState::Active;
        Property form;if(SUCCEEDED(properties->GetValue(PKEY_AudioEndpoint_FormFactor,&form.value))&&form.value.vt==VT_UI4){
            switch(form.value.ulVal){
            case LineLevel:host.inputKind=AudioInputKind::Line;break;
            case Microphone:case Headset:case Handset:host.inputKind=AudioInputKind::Microphone;break;
            case SPDIF:case UnknownDigitalPassthrough:case DigitalAudioDisplayDevice:host.inputKind=AudioInputKind::Digital;break;
            default:break;
            }
        }
        result.push_back(std::move(host));
    }
    const auto priority=[](AudioInputKind kind){return kind==AudioInputKind::Line?0:kind==AudioInputKind::Microphone?MicrophonePriority:1;};
    std::stable_sort(result.begin(),result.end(),[&](const auto& a,const auto& b){return priority(a.inputKind)<priority(b.inputKind);});
    return result;
}
}
OpticalCapabilities parseOpticalConfiguration(const QByteArray& bytes){
    const auto fail=[](){throw std::runtime_error(OpticalQueryFailed);};
    if(bytes.size()<sizeof(GET_CONFIGURATION_HEADER))fail();
    const auto* data=reinterpret_cast<const unsigned char*>(bytes.constData());
    quint64 length=0;for(int index=0;index<ConfigurationLengthBytes;++index)length=(length<<BigEndianShift)|data[index];
    length+=ConfigurationLengthBytes;if(length>static_cast<quint64>(bytes.size())||length<sizeof(GET_CONFIGURATION_HEADER))fail();
    OpticalCapabilities result;quint64 offset=sizeof(GET_CONFIGURATION_HEADER);
    while(offset<length){
        if(length-offset<sizeof(FEATURE_HEADER))fail();
        FEATURE_HEADER header{};std::memcpy(&header,data+offset,sizeof(header));
        const auto size=sizeof(header)+header.AdditionalLength;if(size>length-offset)fail();
        const auto code=(header.FeatureCode[0]<<BigEndianShift)|header.FeatureCode[1];
        result.features.push_back(code);
        if(code==FeatureProfileList){
            if(header.AdditionalLength%sizeof(FEATURE_DATA_PROFILE_LIST_EX))fail();
            for(quint64 item=offset+sizeof(header);item<offset+size;item+=sizeof(FEATURE_DATA_PROFILE_LIST_EX)){
                FEATURE_DATA_PROFILE_LIST_EX profile{};std::memcpy(&profile,data+item,sizeof(profile));
                result.profiles.push_back((profile.ProfileNumber[0]<<BigEndianShift)|profile.ProfileNumber[1]);
            }
        }
        if(code==FeatureCdRead){result.cdRead=true;if(size>=sizeof(FEATURE_DATA_CD_READ)){FEATURE_DATA_CD_READ cd{};std::memcpy(&cd,data+offset,sizeof(cd));result.cdDetailsKnown=true;result.c2=cd.C2ErrorData;result.cdText=cd.CDText;}}
        if(code==FeatureDvdRead)result.dvdRead=true;
        if(code==FeatureBDRead)result.bdRead=true;
        offset+=size;
    }
    return result;
}
OpticalCapabilities queryOpticalCapabilities(const QString& address){
    const auto fail=[](){throw std::runtime_error(OpticalQueryFailed);};
    std::unique_ptr<void,HandleCloser> device(CreateFileW(reinterpret_cast<LPCWSTR>(address.utf16()),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,nullptr));
    if(device.get()==INVALID_HANDLE_VALUE)fail();
    std::unique_ptr<void,HandleCloser> event(CreateEventW(nullptr,TRUE,FALSE,nullptr));if(!event)fail();
    OVERLAPPED operation{};operation.hEvent=event.get();
    GET_CONFIGURATION_IOCTL_INPUT request{};request.Feature=FeatureProfileList;request.RequestType=SCSI_GET_CONFIGURATION_REQUEST_TYPE_ALL;
    QByteArray response(ConfigurationBufferSize,0);DWORD received=0;
    if(!DeviceIoControl(device.get(),IOCTL_CDROM_GET_CONFIGURATION,&request,sizeof(request),response.data(),static_cast<DWORD>(response.size()),&received,&operation)){
        if(GetLastError()!=ERROR_IO_PENDING)fail();
        if(WaitForSingleObject(event.get(),ConfigurationTimeoutMs)!=WAIT_OBJECT_0){CancelIoEx(device.get(),&operation);GetOverlappedResult(device.get(),&operation,&received,TRUE);fail();}
        if(!GetOverlappedResult(device.get(),&operation,&received,FALSE))fail();
    }
    response.resize(received);return parseOpticalConfiguration(response);
}
QVector<DiskController> diskControllers(){
    QVector<DiskController> result;
    for(const auto* category:{&GUID_DEVCLASS_HDC,&GUID_DEVCLASS_SCSIADAPTER}){
        const auto handle=SetupDiGetClassDevsW(category,nullptr,nullptr,DIGCF_PRESENT);if(handle==INVALID_HANDLE_VALUE)throw std::runtime_error(Failure);
        std::unique_ptr<void,DeviceListCloser> list(handle);
        for(DWORD index=0;;++index){SP_DEVINFO_DATA info{};info.cbSize=sizeof(info);if(!SetupDiEnumDeviceInfo(handle,index,&info))break;result.push_back(controllerNode(info.DevInst));}
    }
    for(const auto& device:classDevices(GUID_DEVCLASS_DISKDRIVE,false))if(!device.parentId.isEmpty()&&std::none_of(result.cbegin(),result.cend(),[&](const auto& controller){return controller.id==device.parentId;}))result.push_back({device.parentId,device.parentName,device.parentLocation,device.parentDriver,device.connection});
    return result;
}
DiskAccessReport queryDiskAccess(const QString& address,bool readSample){
    std::unique_ptr<void,HandleCloser> device(CreateFileW(reinterpret_cast<LPCWSTR>(address.utf16()),0,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,nullptr));
    if(device.get()==INVALID_HANDLE_VALUE)throw std::runtime_error(DiskQueryFailed);
    DiskAccessReport report;DWORD received=0;
    STORAGE_PROPERTY_QUERY query{};query.QueryType=PropertyStandardQuery;
    QByteArray storage(BufferSize,0);
    query.PropertyId=StorageDeviceProperty;
    if(control(device.get(),IOCTL_STORAGE_QUERY_PROPERTY,&query,sizeof(query),storage.data(),static_cast<DWORD>(storage.size()),received)&&received>=sizeof(STORAGE_DEVICE_DESCRIPTOR)){
        STORAGE_DEVICE_DESCRIPTOR description{};std::memcpy(&description,storage.constData(),sizeof(description));report.connection=busName(description.BusType);
    }
    query.PropertyId=StorageAdapterProperty;
    if(control(device.get(),IOCTL_STORAGE_QUERY_PROPERTY,&query,sizeof(query),storage.data(),static_cast<DWORD>(storage.size()),received)&&received>=sizeof(STORAGE_ADAPTER_DESCRIPTOR)){
        STORAGE_ADAPTER_DESCRIPTOR adapter{};std::memcpy(&adapter,storage.constData(),sizeof(adapter));report.adapterPropertiesKnown=true;report.maximumTransferBytes=adapter.MaximumTransferLength;report.alignmentMask=adapter.AlignmentMask;report.commandQueueing=adapter.CommandQueueing!=FALSE;
        report.maximumPhysicalPages=adapter.MaximumPhysicalPages;report.adapterUsesPio=adapter.AdapterUsesPio!=FALSE;report.acceleratedTransfer=adapter.AcceleratedTransfer!=FALSE;report.busMajorVersion=adapter.BusMajorVersion;report.busMinorVersion=adapter.BusMinorVersion;
    }
    SCSI_ADDRESS addressInfo{};addressInfo.Length=sizeof(addressInfo);
    if(control(device.get(),IOCTL_SCSI_GET_ADDRESS,nullptr,0,&addressInfo,sizeof(addressInfo),received)&&received>=sizeof(addressInfo)){
        report.addressKnown=true;report.portNumber=addressInfo.PortNumber;report.pathId=addressInfo.PathId;report.targetId=addressInfo.TargetId;report.lun=addressInfo.Lun;
    }
    if(control(device.get(),IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,nullptr,0,storage.data(),static_cast<DWORD>(storage.size()),received)&&received>=sizeof(DISK_GEOMETRY_EX)){
        DISK_GEOMETRY_EX geometry{};std::memcpy(&geometry,storage.constData(),sizeof(geometry));if(geometry.DiskSize.QuadPart>0)report.capacityBytes=geometry.DiskSize.QuadPart;report.logicalSectorBytes=geometry.Geometry.BytesPerSector;
    }
    if(!report.logicalSectorBytes){
        DISK_GEOMETRY geometry{};if(control(device.get(),IOCTL_DISK_GET_DRIVE_GEOMETRY,nullptr,0,&geometry,sizeof(geometry),received)&&received>=sizeof(geometry))report.logicalSectorBytes=geometry.BytesPerSector;
    }
    query.PropertyId=StorageAccessAlignmentProperty;
    STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR alignment{};
    if(control(device.get(),IOCTL_STORAGE_QUERY_PROPERTY,&query,sizeof(query),&alignment,sizeof(alignment),received)&&received>=sizeof(alignment)){report.logicalSectorBytes=alignment.BytesPerLogicalSector;report.physicalSectorBytes=alignment.BytesPerPhysicalSector;}
    if(!readSample)return report;
    if(!report.logicalSectorBytes||report.capacityBytes<report.logicalSectorBytes||report.logicalSectorBytes>MaximumLogicalSectorBytes){report.readState=DiskReadState::MediaInformationUnavailable;return report;}
    device.reset(CreateFileW(reinterpret_cast<LPCWSTR>(address.utf16()),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,nullptr));
    if(device.get()==INVALID_HANDLE_VALUE){report.nativeError=GetLastError();report.readState=report.nativeError==ERROR_ACCESS_DENIED?DiskReadState::AccessDenied:DiskReadState::Failed;return report;}
    std::unique_ptr<void,HandleCloser> event(CreateEventW(nullptr,TRUE,FALSE,nullptr));if(!event){report.readState=DiskReadState::Failed;return report;}
    QByteArray sample(report.logicalSectorBytes,0);
    for(const auto sector:{quint64(0)}){
        report.failedSector=sector;
        const auto offset=sector*report.logicalSectorBytes;OVERLAPPED operation{};operation.hEvent=event.get();operation.Offset=static_cast<DWORD>(offset);operation.OffsetHigh=static_cast<DWORD>(offset>>OffsetHighShift);ResetEvent(event.get());received=0;
        const auto immediate=ReadFile(device.get(),sample.data(),static_cast<DWORD>(sample.size()),&received,&operation);
        if(!immediate){
            const auto error=GetLastError();if(error!=ERROR_IO_PENDING){report.nativeError=error;report.readState=DiskReadState::Failed;return report;}
            if(WaitForSingleObject(event.get(),ConfigurationTimeoutMs)!=WAIT_OBJECT_0){CancelIoEx(device.get(),&operation);GetOverlappedResult(device.get(),&operation,&received,TRUE);report.nativeError=ERROR_TIMEOUT;report.readState=DiskReadState::Failed;return report;}
            if(!GetOverlappedResult(device.get(),&operation,&received,FALSE)){report.nativeError=GetLastError();report.readState=DiskReadState::Failed;return report;}
        }
        if(received!=report.logicalSectorBytes){report.readState=DiskReadState::Failed;return report;}++report.sampledSectors;report.rawAccessVerified=true;
    }
    report.readState=DiskReadState::Passed;return report;
}
QVector<HostDevice> discoverHardware(const QString& category) {
    if(category==KryoFlux)return classDevices(GUID_DEVCLASS_USB,false,true);
    if(category==Floppy)return classDevices(GUID_DEVCLASS_PORTS,true);
    if(category==Optical)return classDevices(GUID_DEVCLASS_CDROM,false);
    if(category==Disks)return classDevices(GUID_DEVCLASS_DISKDRIVE,false);
    if(category==Tape)return classDevices(GUID_DEVCLASS_TAPEDRIVE,false);
    if(category==Audio)return audioDevices();
    return {}; // Cartridge interfaces require their own adapter, or an explicit manual connection.
}
}
