#include <RetroMediaLab/MediaEngine/AudioConversion.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include "Constants.h"
#include "../Sources/AtomicImageWriter.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QTemporaryFile>
#include <array>

namespace RetroMediaLab::MediaEngine {
namespace {
using namespace AudioConversionConstants;
QString text(std::string_view value){return QString::fromUtf8(value.data(),static_cast<qsizetype>(value.size()));}
QString pathText(const std::filesystem::path& path){return QString::fromStdU16String(path.u16string());}
struct ProcessCleanup {
    QProcess& process;
    ~ProcessCleanup(){if(process.state()!=QProcess::NotRunning){process.kill();process.waitForFinished(-1);}}
};
}
AudioConversionService::AudioConversionService(std::filesystem::path executable):executable_(std::move(executable)){}
std::filesystem::path AudioConversionService::executable()const{
    if(!executable_.empty())return executable_;
#ifdef Q_OS_WIN
    const auto name=WindowsExecutable;
#else
    const auto name=UnixExecutable;
#endif
    const auto bundled=QDir(QCoreApplication::applicationDirPath()).filePath(text(Directory)+QDir::separator()+text(name));
    return std::filesystem::path(bundled.toStdU16String());
}
std::filesystem::path AudioConversionService::convert(const AudioConversionRequest& request,std::stop_token token)const {
    checkCancellation(token);const auto selected=static_cast<std::size_t>(request.format);
    if(request.input.empty()||request.output.empty()||selected>=Targets.size()
        ||(request.sampleRate&&(*request.sampleRate<MinimumRate||*request.sampleRate>MaximumRate))
        ||(request.channels&&(*request.channels<=0||*request.channels>MaximumChannels))
        ||(request.bitRate&&(*request.bitRate<=0||*request.bitRate>MaximumBitRate)))throw MediaError(DiagnosticCode::FfmpegRequest);
    const QFileInfo input(pathText(request.input)),output(pathText(request.output));
    if(!input.isFile())throw MediaError(DiagnosticCode::FfmpegInput,{input.absoluteFilePath().toUtf8().toStdString()});
    if(output.exists()){
        std::error_code error;const auto same=std::filesystem::equivalent(request.input,request.output,error);
        if(same||!request.overwrite||!output.isFile())throw MediaError(DiagnosticCode::FfmpegOutput,{output.absoluteFilePath().toUtf8().toStdString()});
    }
    const auto program=executable();if(!QFileInfo(pathText(program)).isFile())throw MediaError(DiagnosticCode::FfmpegMissing,{pathText(program).toUtf8().toStdString()});
    if(!QDir().mkpath(output.absolutePath()))throw MediaError(DiagnosticCode::FfmpegOutput,{output.absoluteFilePath().toUtf8().toStdString()});
    QTemporaryFile temporary(QDir(output.absolutePath()).filePath(text(TemporaryPattern)));
    if(!temporary.open())throw MediaError(DiagnosticCode::FfmpegOutput,{output.absoluteFilePath().toUtf8().toStdString()});
    const auto temporaryPath=temporary.fileName();temporary.close();
    const auto& target=Targets[selected];
    QStringList arguments{text(HideBanner),text(LogLevel),text(ErrorLevel),text(NoStdin),text(NoStats),text(Input),input.absoluteFilePath(),text(Map),text(FirstAudio),text(NoVideo),text(NoSubtitles),text(NoData),text(Codec),text(target.codec),text(Muxer),text(target.muxer)};
    if(request.sampleRate)arguments<<text(Rate)<<QString::number(*request.sampleRate);
    if(request.channels)arguments<<text(Channels)<<QString::number(*request.channels);
    if(request.bitRate)arguments<<text(BitRate)<<QString::number(*request.bitRate);
    arguments<<text(Overwrite)<<temporaryPath;
    QProcess process;ProcessCleanup cleanup{process};
#ifdef Q_OS_WIN
    process.setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments* args){args->flags|=CreateNoWindow;});
#endif
    process.setProgram(pathText(program));process.setArguments(arguments);process.start();
    if(!process.waitForStarted(StartTimeoutMs))throw MediaError(DiagnosticCode::FfmpegStart,{std::to_string(static_cast<int>(process.error()))});
    process.closeWriteChannel();
    while(process.state()!=QProcess::NotRunning){checkCancellation(token);process.waitForFinished(WaitIntervalMs);process.readAllStandardError();process.readAllStandardOutput();}
    checkCancellation(token);
    if(process.exitStatus()!=QProcess::NormalExit||process.exitCode()!=0)throw MediaError(DiagnosticCode::FfmpegFailed,{std::to_string(process.exitCode())});
    if(QFileInfo(temporaryPath).size()<=0)throw MediaError(DiagnosticCode::FfmpegFailed,{std::to_string(process.exitCode())});
    if(!request.overwrite){
        if(!temporary.rename(output.absoluteFilePath()))throw MediaError(DiagnosticCode::FfmpegOutput,{output.absoluteFilePath().toUtf8().toStdString()});
        temporary.setAutoRemove(false);
    }else{
        QFile converted(temporaryPath);if(!converted.open(QIODevice::ReadOnly))throw MediaError(DiagnosticCode::FfmpegOutput,{output.absoluteFilePath().toUtf8().toStdString()});
        writeAtomicImage(request.output,[&](const ImageOutput& write,std::stop_token stop){std::array<std::uint8_t,CopyChunk> bytes;
            for(;;){checkCancellation(stop);const auto count=converted.read(reinterpret_cast<char*>(bytes.data()),static_cast<qint64>(bytes.size()));
                if(count<0)throw MediaError(DiagnosticCode::FfmpegOutput,{output.absoluteFilePath().toUtf8().toStdString()});if(!count)break;write({bytes.data(),static_cast<std::size_t>(count)});}
        },token);
    }
    return request.output;
}
}
