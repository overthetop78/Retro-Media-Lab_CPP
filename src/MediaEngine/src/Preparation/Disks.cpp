#include <RetroMediaLab/MediaEngine/MediaPreparation.h>
#include "../Composition/Runtime.h"
#include "../Models/Sectors/Representation.h"
#include "../Images/Creation/Blank.h"
#include "../Formats/HardDisk/MfmEmu/Adapter.h"
#include "../Formats/HardDisk/MfmEmu/Constants.h"
#include "../Formats/HardDisk/Acorn/Container.h"
#include "../Formats/HardDisk/Acorn/Constants.h"
#include "../Sources/RandomAccessSources.h"
#include <RetroMediaLab/MediaFileSystems/Adfs.h>
#include <RetroMediaLab/MediaFileSystems/FileSystemIds.h>
#include <limits>
namespace RetroMediaLab::MediaEngine {
namespace M=MediaFileSystems;namespace A=AcornWinchester::Constants;
namespace {
std::shared_ptr<const MediaSource> source(const std::filesystem::path& path){SourceOptions options;const auto utf8=path.u8string();options.primaryPath.assign(utf8.begin(),utf8.end());return std::make_shared<MediaSource>(std::move(options));}
}
std::future<void> createBlankDiskAsync(std::filesystem::path path,std::string format,std::stop_token token){
    return std::async(std::launch::async,[path=std::move(path),format=std::move(format),token]{
        checkCancellation(token);auto image=BlankSectorImageFactory::create(format,token);auto document=std::make_shared<MediaDocument>(source(path),format,MediaKind::Floppy,std::make_shared<SectorRepresentation>(std::move(image)));
        RuntimeDetails::defaultServices()->writing.writingService().writeAsync(std::move(document),path,format,token).get();
    });
}
std::future<void> createHardDiskAsync(std::filesystem::path path,std::string format,HardDiskGeometry geometry,std::optional<std::string> fileSystemId,std::stop_token token,bool preallocate){
    return std::async(std::launch::async,[path=std::move(path),format=std::move(format),geometry,fileSystemId=std::move(fileSystemId),token,preallocate]{
        checkCancellation(token);if(format==MfmEmu::Constants::FormatId){MfmEmu::createAsync(path,geometry,fileSystemId,token,preallocate).get();return;}
        if(format!=A::FormatId)throw MediaError(DiagnosticCode::DiskPreparationRequest);if(!AcornWinchester::supportsGeometry(geometry))throw MediaError(DiagnosticCode::AcornDscGeometry);
        std::vector<DataRange> ranges;std::int64_t position=0;
        if(fileSystemId){
            if(geometry.sectorSize()!=A::SectorSize||*fileSystemId!=M::FileSystemIds::AcornAdfs||geometry.logicalSectorCount()>std::numeric_limits<std::int32_t>::max())throw MediaError(DiagnosticCode::DiskPreparationRequest);
            const auto header=M::Adfs::createBbcHeader(static_cast<std::uint32_t>(geometry.logicalSectorCount()));position=static_cast<std::int64_t>(header.size());ranges.emplace_back(0,position,DataRangeKind::Stored,memorySource(header));
        }
        if(position<geometry.capacity())ranges.emplace_back(position,geometry.capacity()-position,DataRangeKind::Zero);
        auto blocks=std::make_shared<BlockImage>(geometry.capacity(),geometry.sectorSize(),std::move(ranges),geometry);auto document=std::make_shared<MediaDocument>(source(path),format,MediaKind::HardDisk,std::move(blocks));
        AcornWinchester::Writer{}.createAsync(std::move(document),path,format,token,preallocate).get();
    });
}
}
