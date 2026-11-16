#include "DiskState.h"
#include <limits>
namespace RetroMediaLab::MediaEngine {
RuntimeDisk::DecodedState::DecodedState(std::shared_ptr<RuntimeDetails::Services> engine,std::shared_ptr<const MediaDocument> source,bool allowed,bool owns):State(std::move(engine),std::move(source),allowed,owns){
    if((document->mediaKind()!=MediaKind::Floppy&&document->mediaKind()!=MediaKind::HardDisk)||(!sectors()&&!blocks()))throw MediaError(DiagnosticCode::RuntimeDiskRequest);
}
RuntimeDisk::DecodedState::~DecodedState()noexcept{try{close();}catch(...) {}}
const MediaFileSystems::SectorImage* RuntimeDisk::DecodedState::sectors()const{if(const auto* value=dynamic_cast<const SectorRepresentation*>(&document->representation()))return &value->image();return nullptr;}
const BlockImage* RuntimeDisk::DecodedState::blocks()const{return dynamic_cast<const BlockImage*>(&document->representation());}
std::int32_t RuntimeDisk::DecodedState::size()const{return sectors()?RuntimeDetails::coordinate(sectors()->geometry().blockSize):blocks()->logicalBlockSize();}
std::int64_t RuntimeDisk::DecodedState::count()const{if(sectors()){if(sectors()->blockCount()>static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()))throw MediaError(DiagnosticCode::RuntimeDiskSector);return static_cast<std::int64_t>(sectors()->blockCount());}return blocks()->logicalBlockCount();}
std::int64_t RuntimeDisk::DecodedState::cylinders()const{return sectors()?sectors()->geometry().cylinders:blocks()->geometry()?blocks()->geometry()->cylinders():0;}
std::int32_t RuntimeDisk::DecodedState::heads()const{return sectors()?RuntimeDetails::coordinate(sectors()->geometry().heads):blocks()->geometry()?blocks()->geometry()->heads():0;}
std::int32_t RuntimeDisk::DecodedState::sectorsPerTrack()const{return sectors()?RuntimeDetails::coordinate(sectors()->geometry().sectorsPerTrack):blocks()->geometry()?blocks()->geometry()->sectorsPerTrack():0;}
void RuntimeDisk::DecodedState::validate(std::int64_t index)const{alive();if(index<0||index>=count())throw MediaError(DiagnosticCode::RuntimeDiskSector);}
bool RuntimeDisk::DecodedState::dirty()const{return !changes.empty()||documentChanged;}
}
