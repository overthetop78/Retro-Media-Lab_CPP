#include "DecodedPcm.h"
#include "Constants.h"
#include "../../Tape/Common/Pcm.h"
#include "../../../Sources/RandomAccessSources.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QAudioDecoder>
#include <QCoreApplication>
#include <QEventLoop>
#include <QIODevice>
#include <QtEndian>
#include <algorithm>
#include <cmath>
#include <limits>
#include <mutex>

namespace RetroMediaLab::MediaEngine::AudioPcm {
namespace {
using namespace Constants;
class SourceDevice final:public QIODevice {
public:
    SourceDevice(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,std::stop_token token):source_(std::move(source)),token_(token){
        if(!source_||source_->length()>static_cast<std::uint64_t>(std::numeric_limits<qint64>::max()))throw MediaError(DiagnosticCode::AudioPcmData);
        open(QIODevice::ReadOnly);
    }
    qint64 size()const override{return static_cast<qint64>(source_->length());}
    bool isSequential()const override{return false;}
    bool seek(qint64 position)override{
        std::lock_guard lock(mutex_);if(position<0||position>size()||!QIODevice::seek(position))return false;cursor_=position;return true;
    }
    void rethrowFailure(){std::lock_guard lock(mutex_);if(failure_)std::rethrow_exception(failure_);}
protected:
    qint64 readData(char* data,qint64 maximum)override {
        std::lock_guard lock(mutex_);
        try{
            checkCancellation(token_);if(maximum<=0)return 0;
            const auto count=std::min(maximum,size()-cursor_);
            source_->readExactly(static_cast<std::uint64_t>(cursor_),{reinterpret_cast<std::uint8_t*>(data),static_cast<std::size_t>(count)},token_);cursor_+=count;return count;
        }catch(...){failure_=std::current_exception();return -1;}
    }
    qint64 writeData(const char*,qint64)override{return -1;}
private:
    std::shared_ptr<const MediaFileSystems::RandomAccessData> source_;std::stop_token token_;qint64 cursor_=0;std::mutex mutex_;std::exception_ptr failure_;
};
void appendBuffer(const QAudioBuffer& buffer,std::vector<std::uint8_t>& bytes,int& channels,int& rate,std::stop_token token){
    if(!buffer.isValid())throw MediaError(DiagnosticCode::AudioPcmData);
    const auto format=buffer.format();const auto count=buffer.sampleCount();
    if(format.channelCount()<=0||format.channelCount()>MaximumAlign/SampleBytes||format.sampleRate()<=0||count<0||count%format.channelCount())throw MediaError(DiagnosticCode::AudioPcmData);
    if(channels&&(channels!=format.channelCount()||rate!=format.sampleRate()))throw MediaError(DiagnosticCode::AudioPcmData);
    channels=format.channelCount();rate=format.sampleRate();
    if(static_cast<std::uint64_t>(count)>(MaximumBuffer-bytes.size())/SampleBytes)throw MediaError(DiagnosticCode::AudioPcmData);
    const auto offset=bytes.size();bytes.resize(offset+static_cast<std::size_t>(count)*SampleBytes);
    for(int index=0;index<count;++index){
        checkCancellation(token);double sample{};
        switch(format.sampleFormat()){
        case QAudioFormat::Float:sample=buffer.constData<float>()[index];break;
        case QAudioFormat::Int16:sample=buffer.constData<qint16>()[index]/Int16Scale;break;
        case QAudioFormat::Int32:sample=buffer.constData<qint32>()[index]/Int32Scale;break;
        case QAudioFormat::UInt8:sample=(int(buffer.constData<quint8>()[index])-UInt8Bias)/UInt8Scale;break;
        default:throw MediaError(DiagnosticCode::AudioPcmData);
        }
        if(!std::isfinite(sample))throw MediaError(DiagnosticCode::AudioPcmData);
        const auto value=static_cast<qint16>(std::clamp(TapePcm::roundEven(sample*PcmMaximum),PcmMinimum,PcmMaximum));
        qToLittleEndian(value,bytes.data()+offset+static_cast<std::size_t>(index)*SampleBytes);
    }
}
}
DecodedPcm decode(std::shared_ptr<const MediaFileSystems::RandomAccessData> source,std::stop_token token){
    checkCancellation(token);if(!QCoreApplication::instance())throw MediaError(DiagnosticCode::AudioPcmRuntime);
    std::vector<std::uint8_t> bytes;int channels=0,rate=0;bool done=false;std::exception_ptr failure;
    SourceDevice device(std::move(source),token);QEventLoop loop;QAudioDecoder decoder;
    if(!decoder.isSupported())throw MediaError(DiagnosticCode::AudioPcmRuntime);
    const auto drain=[&]{try{while(decoder.bufferAvailable())appendBuffer(decoder.read(),bytes,channels,rate,token);}catch(...){failure=std::current_exception();done=true;loop.quit();}};
    QObject::connect(&decoder,&QAudioDecoder::bufferReady,&loop,drain);
    QObject::connect(&decoder,&QAudioDecoder::finished,&loop,[&,drain]{drain();done=true;loop.quit();});
    QObject::connect(&decoder,qOverload<QAudioDecoder::Error>(&QAudioDecoder::error),&loop,[&](QAudioDecoder::Error code){failure=std::make_exception_ptr(MediaError(DiagnosticCode::AudioPcmDecode,{std::to_string(static_cast<int>(code))}));done=true;loop.quit();});
    struct Cleanup {QAudioDecoder& decoder;QEventLoop& loop;~Cleanup(){QObject::disconnect(&decoder,nullptr,&loop,nullptr);decoder.stop();}} cleanup{decoder,loop};
    std::stop_callback cancellation(token,[&]{QMetaObject::invokeMethod(&loop,[&]{done=true;loop.quit();},Qt::QueuedConnection);});
    decoder.setSourceDevice(&device);decoder.start();if(!done)loop.exec();
    QObject::disconnect(&decoder,nullptr,&loop,nullptr);decoder.stop();
    checkCancellation(token);device.rethrowFailure();if(failure)std::rethrow_exception(failure);
    if(bytes.empty()||!channels||!rate)throw MediaError(DiagnosticCode::AudioPcmData);
    return {memorySource(bytes),channels,rate};
}
}
