#include "Internal.h"
#include <limits>
namespace RetroMediaLab::MediaFileSystems::BbcDfs::Internal {
using namespace Constants;
CatalogueView::CatalogueView(const SectorImage& source,std::optional<std::uint32_t> head)
    :source_(source),head_(head),geometry_(source.geometry()),count_(source.blockCount()) {
    if(!head_)return;
    if(geometry_.formatId==Jfd || geometry_.formatId==AcornDfsDoubleSided || geometry_.formatId==AcornDfsDoubleSided80)
        geometry_.formatId=geometry_.cylinders>FortyCylinders?AcornDfsSingleSided80:AcornDfsSingleSided;
    geometry_.heads=1;count_=std::uint64_t(geometry_.cylinders)*geometry_.sectorsPerTrack;
    if(!geometry_.blockSize || count_>std::uint64_t(std::numeric_limits<std::int64_t>::max())/(std::uint64_t(geometry_.blockSize)*source_.geometry().heads))
        throw MediaError(DiagnosticCode::SectorInvalidGeometry,{std::string(SideGeometry),std::to_string(count_)});
}
std::shared_ptr<const SectorBlock> CatalogueView::block(std::uint64_t logical,std::stop_token token)const {
    checkCancellation(token);if(logical>=count_)return {};
    const auto original=head_?(logical/geometry_.sectorsPerTrack*source_.geometry().heads+*head_)*geometry_.sectorsPerTrack+logical%geometry_.sectorsPerTrack:logical;
    // The parser borrows the original physical record. Only its data and integrity are consumed.
    return source_.block(original,token);
}
bool canReadSides(const SectorImage& image,std::stop_token token) {
    for(std::uint32_t head=0;head<image.geometry().heads;++head)if(canReadSingle(CatalogueView(image,head),token))return true;
    return false;
}
FileSystemVolume readSides(const SectorImage& image,std::stop_token token) {
    FileSystemVolume result;result.fileSystemId=FileSystemId;result.fileSystemDisplayName=FileSystemId;
    for(std::uint32_t head=0;head<image.geometry().heads;++head) {
        checkCancellation(token);const auto name=std::to_string(head*TwoHeads);
        FileSystemEntry entry;entry.name=name;entry.kind=EntryKind::Directory;entry.storageReference=head;
        const CatalogueView side(image,head);
        try {
            auto volume=readSingle(side,token);entry.comment=volume.name;entry.children=std::move(volume.entries);entry.metadataValid=volume.freeSpaceKnown;if(!volume.freeSpaceKnown)entry.dataValid=false;
            result.capacity+=volume.capacity;result.freeBytes+=volume.freeBytes;
            for(auto& warning:volume.warnings){warning.context.insert(warning.context.begin(),name);result.warnings.push_back(std::move(warning));}
        } catch(const MediaError& error) {
            if(error.kind()!=ErrorKind::InvalidData)throw;
            auto diagnostic=error.diagnostic();diagnostic.context.insert(diagnostic.context.begin(),name);
            result.warnings.push_back(diagnostic);entry.diagnostics.push_back(std::move(diagnostic));entry.metadataValid=false;
            result.capacity+=side.capacity();
        }
        result.entries.push_back(std::move(entry));
    }
    result.freeSpaceKnown=result.warnings.empty();return result;
}
}
