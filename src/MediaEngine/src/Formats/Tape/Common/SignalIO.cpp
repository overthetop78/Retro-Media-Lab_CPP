#include "Signal.h"
#include "SignalBlocks.h"
#include "SignalIoConstants.h"
#include "../Uef/Reader.h"
#include "../Wav/Reader.h"
#include "../Csw/Reader.h"
#include "../Raw/Reader.h"
#include "../AtomTap/Reader.h"
#include "../../../Composition/SequentialMedia.h"
#include "../../../Images/RecognitionContext.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QString>
namespace RetroMediaLab::MediaEngine::TapeSignal {
namespace {
using namespace IoConstants;
std::string extension(const std::filesystem::path& path){return QString::fromStdU16String(path.extension().u16string()).toLower().toStdString();}
std::shared_ptr<const MediaSource> source(const std::filesystem::path& path,Parameters parameters){SourceOptions options;options.primaryPath=QString::fromStdU16String(path.u16string()).toStdString();options.parameters=std::move(parameters);return std::make_shared<MediaSource>(std::move(options));}
}
std::span<const std::string_view> writableExtensions(){return Writable;}
void write(const std::filesystem::path& path,const MediaTapeSignal& signal,Parameters parameters,std::stop_token token){
    checkCancellation(token);const auto suffix=extension(path);auto owner=source(path,suffix==RawExtension?std::move(parameters):Parameters{});
    if(suffix==UefExtension){auto document=toUef(owner,signal,token);Uef::Writer{}.writeAsync(document,path,std::string(TapeFormatIds::Uef),token).get();return;}
    std::shared_ptr<const ImageWriter> writer;std::string_view format;
    if(suffix==WaveExtension){writer=std::make_shared<WavTape::Writer>();format=TapeFormatIds::Wav;}
    else if(suffix==CswExtension){writer=std::make_shared<Csw::Writer>();format=TapeFormatIds::Csw;}
    else if(suffix==RawExtension){writer=std::make_shared<RawTape::Writer>();format=TapeFormatIds::Raw;}
    else throw MediaError(DiagnosticCode::TapeSignalFormat,{suffix});
    const auto document=TapePcm::signalDocument(owner,std::string(format),signal,token);writer->writeAsync(document,path,std::string(format),token).get();
}
MediaTapeSignal read(const std::filesystem::path& path,std::optional<std::string> machine,Parameters parameters,std::stop_token token){
    checkCancellation(token);const auto suffix=extension(path);std::shared_ptr<const ImageReader> reader;
    if(suffix==UefExtension)reader=std::make_shared<Uef::Reader>();
    else if(suffix==WaveExtension)reader=std::make_shared<WavTape::Reader>();
    else if(suffix==CswExtension)reader=std::make_shared<Csw::Reader>();
    else if(suffix==RawExtension)reader=std::make_shared<RawTape::Reader>();
    else if(suffix==AtomExtension)reader=std::make_shared<AtomTap::Reader>();
    else throw MediaError(DiagnosticCode::TapeSignalFormat,{suffix});
    const auto document=reader->readAsync(RecognitionContext(source(path,std::move(parameters))),token).get();
    const auto& image=dynamic_cast<const SequentialImage&>(document->representation());
    std::map<std::int64_t,int> positions;
    auto signal=[&]{if(suffix==UefExtension||suffix==AtomExtension){auto result=fromUef(image,token);positions=std::move(result.sourcePositions);return std::move(result.signal);}return pcm(*document,token);}();
    if(!machine||signal.samples().empty())return signal;
    const auto composition=SequentialMediaComposition::createDefault();const auto& decoders=composition.decoders();
    if(decoders.findCompatible(*document,machine,token).empty())return signal;
    auto decoded=decoders.decodeAsync(document,machine,token).get();
    if(decoded.blocks().empty()&&document->formatId()==TapeFormatIds::Uef){
        const auto wave=TapePcm::signalDocument(document->mediaSource(),std::string(TapeFormatIds::Wav),signal,token);
        decoded=decoders.decodeAsync(wave,machine,token).get();
    }
    return associateBlocks(signal,decoded,image,positions,token);
}
}
