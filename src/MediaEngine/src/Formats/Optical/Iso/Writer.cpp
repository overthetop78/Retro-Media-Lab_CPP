#include "Writer.h"
#include "Constants.h"
#include "../../FormatIds.h"
#include "../../../Models/Optical/Image.h"
#include "../../../Images/RecognitionContext.h"
#include "../../../Sources/AtomicImageWriter.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine::OpticalIso {
namespace M=MediaFileSystems;
using namespace OpticalIsoConstants;
namespace {
bool profile(const OpticalImage& image){
    const auto& options=image.options();if(!options.tracks||options.tracks->size()!=1||!image.sessions()||image.sessions()->size()!=1||options.layerCount||options.faceCount)return false;
    const auto& track=options.tracks->front()->options();return track.sessionNumber==1&&track.trackNumber==1&&track.firstSector==0&&track.mode==OpticalTrackMode::Mode1Data2048&&track.storedSectorSize==SectorSize&&track.userDataOffset==0&&track.userDataLength==SectorSize&&!track.subchannelSource&&track.pregapSectors==0&&track.storedPregapSectors==0&&track.postgapSectors==0&&track.flags.empty()&&!track.catalogNumber&&!track.isrc&&std::all_of(track.indexes.begin(),track.indexes.end(),[](const auto& index){return index.number()==1&&index.relativeSector()==0;});
}
}
const WriterCapabilities& Writer::capabilities()const{static const WriterCapabilities value{std::string(WriterId),{std::string(Formats::OpticalIso)},{std::string(IsoExtension),std::string(GcmExtension)},{RepresentationKind::OpticalTracks},false};return value;}
bool Writer::canWrite(const MediaDocument& document,std::string_view format,std::string_view extension)const{const auto* image=dynamic_cast<const OpticalImage*>(&document.representation());return supportsTarget(format,extension)&&document.mediaKind()==MediaKind::Optical&&image&&profile(*image);}
std::future<std::vector<std::filesystem::path>> Writer::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path output,std::string target,std::stop_token token)const{return std::async(std::launch::deferred,[document=std::move(document),output=std::move(output),target=std::move(target),token]{
    const auto extension=output.extension().u8string();if(!document||!Writer{}.canWrite(*document,target,std::string(extension.begin(),extension.end())))throw M::MediaError(M::DiagnosticCode::IsoWriteProfile);
    const auto& image=dynamic_cast<const OpticalImage&>(document->representation());const auto track=image.options().tracks->front();
    writeAtomicImage(output,[&](const ImageOutput& sink,std::stop_token current){for(std::int64_t sector=0;sector<track->sectorCount();++sector){M::checkCancellation(current);const auto bytes=track->readStoredSectorAsync(sector,current).get();sink(bytes);}},token);
    return std::vector<std::filesystem::path>{output};
});}
}
