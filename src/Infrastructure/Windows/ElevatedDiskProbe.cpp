#include "Hardware/DiskProbeService.h"
#include <HardwareDiscoveryConstants.h>
#include <StorageProbeConstants.h>
#include <ProcessConstants.h>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QLocalServer>
#include <QLocalSocket>
#include <QUuid>
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#include <memory>
#include <stdexcept>

namespace RetroMediaLab::Infrastructure {
namespace {
using namespace StorageProbeConstants;
struct CloseHandleOnExit {void operator()(void* handle)const noexcept{if(handle&&handle!=INVALID_HANDLE_VALUE)CloseHandle(handle);}};
struct ComSession {
    HRESULT result=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED|COINIT_DISABLE_OLE1DDE);
    ~ComSession(){if(SUCCEEDED(result))CoUninitialize();}
};
[[noreturn]] void exchangeFailed(){throw std::runtime_error(HardwareDiscoveryConstants::DiskQueryFailed);}
void send(QLocalSocket& socket,const QJsonObject& object){
    const auto bytes=QJsonDocument(object).toJson(QJsonDocument::Compact)+LineEnd;
    if(bytes.size()>MaximumMessageBytes||socket.write(bytes)!=bytes.size())exchangeFailed();
    QElapsedTimer timer;timer.start();
    while(socket.bytesToWrite()){
        if(timer.elapsed()>=ConnectionTimeoutMs||!socket.waitForBytesWritten(ConnectionTimeoutMs-static_cast<int>(timer.elapsed())))exchangeFailed();
    }
}
QJsonObject receive(QLocalSocket& socket){
    socket.setReadBufferSize(MaximumMessageBytes);QElapsedTimer timer;timer.start();
    while(!socket.canReadLine()){
        if(socket.bytesAvailable()>=MaximumMessageBytes||timer.elapsed()>=ExchangeTimeoutMs)exchangeFailed();
        if(!socket.waitForReadyRead(WaitSliceMs)&&socket.state()!=QLocalSocket::ConnectedState&&!socket.canReadLine())exchangeFailed();
    }
    QJsonParseError error;const auto document=QJsonDocument::fromJson(socket.readLine(MaximumMessageBytes),&error);
    if(error.error!=QJsonParseError::NoError||!document.isObject())exchangeFailed();
    return document.object();
}
}

DiskAccessReport requestElevatedDiskProbe(const QString& controller,const QString& disk,DiskAccessReport fallback){
    using namespace StorageProbeConstants;
    fallback.elevation=DiskElevationState::Failed;
    try{
        QLocalServer server;
        server.setSocketOptions(QLocalServer::UserAccessOption);
        const auto name=QString::fromLatin1(PipePrefix)+QUuid::createUuid().toString(QUuid::WithoutBraces);
        if(!server.listen(name))return fallback;
        const auto executable=QCoreApplication::applicationFilePath().toStdWString();
        const auto arguments=QString::fromLatin1(ArgumentsFormat).arg(QString::fromLatin1(Mode),name).toStdWString();
        ComSession com;
        if(FAILED(com.result))return fallback;
        SHELLEXECUTEINFOW launch{};launch.cbSize=sizeof(launch);
        launch.fMask=SEE_MASK_NOCLOSEPROCESS|SEE_MASK_NOASYNC|SEE_MASK_FLAG_NO_UI;
        launch.lpVerb=RunAs;launch.lpFile=executable.c_str();launch.lpParameters=arguments.c_str();launch.nShow=SW_HIDE;
        if(!ShellExecuteExW(&launch)){
            if(GetLastError()==ERROR_CANCELLED)fallback.elevation=DiskElevationState::Cancelled;
            return fallback;
        }
        std::unique_ptr<void,CloseHandleOnExit> process(launch.hProcess);
        if(!process||!server.waitForNewConnection(ConnectionTimeoutMs))return fallback;
        std::unique_ptr<QLocalSocket> socket(server.nextPendingConnection());
        if(!socket)return fallback;
        send(*socket,{{Controller,controller},{Disk,disk}});
        const auto response=receive(*socket);
        if(response.value(Controller).toString()!=controller||response.value(Disk).toString()!=disk||!response.value(Report).isObject())return fallback;
        auto result=diskProbeReportFromJson(response.value(Report).toObject());
        result.elevation=DiskElevationState::Completed;
        return result;
    }catch(...){return fallback;}
}

extern "C" RETROMEDIALAB_INFRASTRUCTURE_API int RetroMediaLabProbe(int argc,char** argv){
    using namespace StorageProbeConstants;
    // This entry point never constructs an application window or accepts commands/paths.
    // The controller and disk must be found together in the current PnP inventory.
    try{
        QCoreApplication app(argc,argv);
        if(argc!=HelperArgumentCount||QString::fromUtf8(argv[1])!=QString::fromLatin1(Mode))return ProcessConstants::Failure;
        const auto name=QString::fromUtf8(argv[PipeArgument]);
        if(!name.startsWith(QString::fromLatin1(PipePrefix))||name.size()>MaximumIdentifierLength)return ProcessConstants::Failure;
        QLocalSocket socket;socket.connectToServer(name);
        if(!socket.waitForConnected(ConnectionTimeoutMs))return ProcessConstants::Failure;
        const auto request=receive(socket);
        const auto controller=request.value(Controller).toString(),disk=request.value(Disk).toString();
        if(controller.isEmpty()||disk.isEmpty()||controller.size()>MaximumIdentifierLength||disk.size()>MaximumIdentifierLength)return ProcessConstants::Failure;
        try{
            const auto report=inspectDiskInterface(controller,disk);
            send(socket,{{Controller,controller},{Disk,disk},{Report,diskProbeReportToJson(report)}});
        }catch(...){send(socket,{{Controller,controller},{Disk,disk},{Error,QString::fromLatin1(HardwareDiscoveryConstants::DiskQueryFailed)}});return ProcessConstants::Failure;}
        return ProcessConstants::Success;
    }catch(...){return ProcessConstants::Failure;}
}
}
