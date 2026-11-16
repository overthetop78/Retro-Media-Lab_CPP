#include "PhysicalStates.h"
#include <QFileInfo>
#include <limits>
namespace RetroMediaLab::MediaEngine {
namespace RuntimeDetails {
std::shared_ptr<Services> defaultServices(){static const auto value=std::make_shared<Services>();return value;}
std::int32_t coordinate(std::int64_t value){if(value<std::numeric_limits<std::int32_t>::min()||value>std::numeric_limits<std::int32_t>::max())throw MediaError(DiagnosticCode::RuntimeDiskSector);return static_cast<std::int32_t>(value);}
DrivePosition position(std::int64_t index,MediaFileSystems::SectorAddress address){if(address.cylinder<0||address.head<0)return {0,0,-1,index};return {coordinate(address.cylinder),coordinate(address.head),coordinate(address.number),-1};}
}
RuntimeDisk::State::State(std::shared_ptr<RuntimeDetails::Services> engine,std::shared_ptr<const MediaDocument> source,bool allowed,bool owns):services(std::move(engine)),document(std::move(source)),ownsMechanics(owns){
    if(!document||!document->mediaSource()->options().drive)throw MediaError(DiagnosticCode::RuntimeDiskRequest);
    const auto path=QString::fromUtf8(document->mediaSource()->primaryPath());const auto extension=std::filesystem::path(path.toStdU16String()).extension().u8string();
    readOnly=!allowed||!QFileInfo(path).isWritable()||!services->writing.registry().canWrite(*document,document->formatId(),std::string(extension.begin(),extension.end()));
}
void RuntimeDisk::State::alive()const{if(closed)throw MediaError(DiagnosticCode::RuntimeDiskClosed);}
void RuntimeDisk::State::writable()const{alive();if(readOnly)throw MediaError(DiagnosticCode::RuntimeDiskReadOnly);}
void RuntimeDisk::State::close(){
    if(closed)return;std::exception_ptr failure;try{flush({});}catch(...){failure=std::current_exception();}
    try{if(ownsMechanics)document->mediaSource()->options().drive->reset();}catch(...){if(!failure)failure=std::current_exception();}
    clear();closed=true;if(failure)std::rethrow_exception(failure);
}
std::optional<std::int64_t> RuntimeDisk::State::findSector(std::int32_t,std::int32_t,std::int32_t)const{alive();return {};}
std::vector<std::int64_t> RuntimeDisk::State::trackSectors(std::int32_t,std::int32_t)const{alive();return {};}
MediaDiskSector RuntimeDisk::State::read(std::int64_t,std::stop_token token){alive();checkCancellation(token);throw MediaError(DiagnosticCode::RuntimeDiskSector);}
void RuntimeDisk::State::write(std::int64_t,std::span<const std::uint8_t>,std::optional<bool>){alive();throw MediaError(DiagnosticCode::RuntimeDiskSector);}
void RuntimeDisk::State::updateMetadata(const std::map<std::string,std::string>& values){alive();if(!values.empty())throw MediaError(DiagnosticCode::RuntimeDiskRequest);}
void RuntimeDisk::State::prepareBlockFormat(const HardDiskGeometry&){alive();throw MediaError(DiagnosticCode::RuntimeDiskRequest);}
void RuntimeDisk::State::formatTrack(std::int32_t,std::int32_t,std::span<const MediaDiskSector>,std::optional<std::int32_t>,std::optional<bool>){alive();throw MediaError(DiagnosticCode::RuntimeTrackProjection);}
std::future<std::unique_ptr<RuntimeDisk>> RuntimeDisk::openAsync(std::filesystem::path path,bool writable,std::optional<std::string> format,std::stop_token token,std::shared_ptr<DriveMechanics> mechanics){
    return std::async(std::launch::async,[services=RuntimeDetails::defaultServices(),path=std::move(path),writable,format=std::move(format),token,mechanics=std::move(mechanics)]{
        checkCancellation(token);SourceOptions options;const auto u8=path.u8string();options.primaryPath.assign(u8.begin(),u8.end());options.requestedFormatId=format;options.drive=mechanics;options.trackDriveAccesses=false;
        auto document=services->reading.readingService().readAsync(std::make_shared<MediaSource>(std::move(options)),token).get();
        bool stateConstructed=false;
        try{
            std::shared_ptr<State> state;
            if(dynamic_cast<const MfmEmu::Representation*>(&document->representation()))state=std::make_shared<MfmState>(services,document,writable,!mechanics);
            else if(dynamic_cast<const FluxRepresentation*>(&document->representation()))state=std::make_shared<FluxState>(services,document,writable,!mechanics);
            else state=std::make_shared<DecodedState>(services,document,writable,!mechanics);
            stateConstructed=true;return std::unique_ptr<RuntimeDisk>(new RuntimeDisk(std::move(state)));
        }
        catch(...){if(!stateConstructed&&!mechanics&&document->mediaSource()->options().drive)try{document->mediaSource()->options().drive->reset();}catch(...){}throw;}
    });
}
RuntimeDisk::~RuntimeDisk()noexcept=default;
std::shared_ptr<DriveMechanics> RuntimeDisk::mechanics()const{std::lock_guard lock(state_->mutex);return state_->document->mediaSource()->options().drive;}
std::string RuntimeDisk::formatId()const{std::lock_guard lock(state_->mutex);return state_->document->formatId();}
std::int32_t RuntimeDisk::sectorSize()const{std::lock_guard lock(state_->mutex);return state_->size();}
std::int64_t RuntimeDisk::sectorCount()const{std::lock_guard lock(state_->mutex);return state_->count();}
std::int64_t RuntimeDisk::cylinders()const{std::lock_guard lock(state_->mutex);return state_->cylinders();}
std::int32_t RuntimeDisk::heads()const{std::lock_guard lock(state_->mutex);return state_->heads();}
std::int32_t RuntimeDisk::sectorsPerTrack()const{std::lock_guard lock(state_->mutex);return state_->sectorsPerTrack();}
bool RuntimeDisk::isReadOnly()const{std::lock_guard lock(state_->mutex);return state_->readOnly;}
bool RuntimeDisk::hasChanges()const{std::lock_guard lock(state_->mutex);return state_->dirty();}
std::map<std::string,std::string> RuntimeDisk::metadata()const{std::lock_guard lock(state_->mutex);return state_->document->metadata();}
std::future<MediaDiskSector> RuntimeDisk::readSectorAsync(std::int64_t index,std::stop_token token)const{return std::async(std::launch::async,[state=state_,index,token]{std::lock_guard lock(state->mutex);return state->read(index,token);});}
void RuntimeDisk::writeSector(std::int64_t index,std::span<const std::uint8_t> data,std::optional<bool> mark){std::lock_guard lock(state_->mutex);state_->write(index,data,mark);}
std::future<void> RuntimeDisk::flushAsync(std::stop_token token){return std::async(std::launch::async,[state=state_,token]{std::lock_guard lock(state->mutex);state->flush(token);});}
void RuntimeDisk::close(){std::lock_guard lock(state_->mutex);state_->close();}
void RuntimeDisk::updateMetadata(const std::map<std::string,std::string>& values){std::lock_guard lock(state_->mutex);state_->updateMetadata(values);}
void RuntimeDisk::prepareBlockFormat(const HardDiskGeometry& geometry){std::lock_guard lock(state_->mutex);state_->prepareBlockFormat(geometry);}
}
