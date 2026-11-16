#include "RecordingDecoder.h"
#include "Constants.h"
#include "../Constants.h"
#include "../../Formats/Common/Audio/Constants.h"
#include <RetroMediaLab/MediaEngine/AudioConversion.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QtEndian>
#include <bit>
#include <cmath>
namespace RetroMediaLab::MediaEngine::MechanicalAudio {
namespace {
QString text(std::string_view value){return QString::fromUtf8(value.data(),static_cast<qsizetype>(value.size()));}
QString pathText(const std::filesystem::path& value){return QString::fromStdU16String(value.u16string());}
struct ProcessCleanup {
    QProcess& process;
    ~ProcessCleanup(){if(process.state()!=QProcess::NotRunning){process.kill();process.waitForFinished(-1);}}
};
}
DecodedRecording decodeRecording(const std::filesystem::path& path,int outputRate,const std::filesystem::path& executable,std::stop_token token){
    namespace C=Constants;namespace A=AudioConversionConstants;
    checkCancellation(token);
    if(outputRate<0)throw MediaError(DiagnosticCode::AudioPcmData);
    QFile input(pathText(path));if(!input.open(QIODevice::ReadOnly))throw MediaError(DiagnosticCode::FfmpegInput,{pathText(path).toUtf8().toStdString()});
    const auto header=input.read(C::FlacHeaderBytes);input.close();
    if(header.size()!=C::FlacHeaderBytes||header.first(C::MetadataHeader)!=text(C::FlacSignature).toUtf8()
        ||(static_cast<unsigned char>(header[C::MetadataHeader])&C::MetadataTypeMask)!=0
        ||(qFromBigEndian<quint32>(header.constData()+C::MetadataHeader)&C::MetadataLengthMask)!=C::StreamInfoBytes)throw MediaError(DiagnosticCode::AudioPcmData);
    const auto format=qFromBigEndian<quint64>(header.constData()+C::StreamRateOffset);
    const auto nativeRate=static_cast<int>(format>>C::StreamRateShift);
    const auto channels=static_cast<int>((format>>C::StreamChannelsShift)&C::ChannelMask)+C::MonoChannels;
    if(nativeRate<=0||(channels!=C::MonoChannels&&channels!=C::StereoChannels))throw MediaError(DiagnosticCode::AudioPcmData);
    const auto rate=outputRate?outputRate:nativeRate;
    const auto program=AudioConversionService(executable).executable();
    if(!QFileInfo(pathText(program)).isFile())throw MediaError(DiagnosticCode::FfmpegMissing,{pathText(program).toUtf8().toStdString()});
    QStringList arguments{text(A::HideBanner),text(A::LogLevel),text(A::ErrorLevel),text(A::NoStdin),text(A::NoStats),text(C::FailOnError),text(A::Input),pathText(path),text(A::Map),text(A::FirstAudio),text(A::NoVideo),text(A::NoSubtitles),text(A::NoData)};
    if(channels==C::StereoChannels)arguments<<text(C::Filter)<<text(C::StereoToMono);
    arguments<<text(A::Rate)<<QString::number(rate)<<text(A::Channels)<<QString::number(C::MonoChannels)<<text(A::Codec)<<text(C::FloatCodec)<<text(A::Muxer)<<text(C::FloatFormat)<<text(C::PipeOutput);
    QProcess process;ProcessCleanup cleanup{process};
#ifdef Q_OS_WIN
    process.setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments* args){args->flags|=A::CreateNoWindow;});
#endif
    process.setProgram(pathText(program));process.setArguments(arguments);process.start();
    if(!process.waitForStarted(A::StartTimeoutMs))throw MediaError(DiagnosticCode::FfmpegStart,{std::to_string(static_cast<int>(process.error()))});
    process.closeWriteChannel();QByteArray bytes;
    const auto drain=[&]{
        const auto block=process.readAllStandardOutput();process.readAllStandardError();
        if(static_cast<std::uint64_t>(bytes.size())+static_cast<std::uint64_t>(block.size())>AudioPcm::Constants::MaximumBuffer)throw MediaError(DiagnosticCode::AudioPcmData);
        bytes.append(block);
    };
    while(process.state()!=QProcess::NotRunning){checkCancellation(token);process.waitForFinished(A::WaitIntervalMs);drain();}
    drain();checkCancellation(token);
    if(process.exitStatus()!=QProcess::NormalExit||process.exitCode()!=0)throw MediaError(DiagnosticCode::FfmpegFailed,{std::to_string(process.exitCode())});
    if(bytes.isEmpty()||bytes.size()%C::FloatBytes)throw MediaError(DiagnosticCode::AudioPcmData);
    DecodedRecording result;result.sampleRate=rate;result.samples.resize(static_cast<std::size_t>(bytes.size()/C::FloatBytes));
    for(std::size_t i=0;i<result.samples.size();++i){checkCancellation(token);const auto value=std::bit_cast<float>(qFromLittleEndian<quint32>(bytes.constData()+i*C::FloatBytes));
        if(!std::isfinite(value))throw MediaError(DiagnosticCode::AudioPcmData);result.samples[i]=value;}
    return result;
}
}
