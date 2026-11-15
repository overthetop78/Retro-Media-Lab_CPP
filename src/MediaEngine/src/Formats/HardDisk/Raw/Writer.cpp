#include "Writer.h"
#include "Format.h"
#include "../../../Models/Blocks/BlockImage.h"
#include "../../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::HardDiskRaw {
const WriterCapabilities& Writer::capabilities()const{static const auto result=[]{WriterCapabilities c;c.id=Constants::WriterId;c.formatIds={std::string(Constants::FormatId)};for(auto extension:Constants::Extensions)c.producedFileExtensions.emplace_back(extension);c.representationKinds={RepresentationKind::Blocks};return c;}();return result;}
bool Writer::canWrite(const MediaDocument& document,std::string_view target,std::string_view extension)const{
    const auto* blocks=dynamic_cast<const BlockImage*>(&document.representation());return supportsTarget(target,extension)&&document.mediaKind()==MediaKind::HardDisk&&blocks&&compatible(blocks->capacity(),blocks->logicalBlockSize())&&blocks->hasCompleteReadableCoverage();
}
std::future<std::vector<std::filesystem::path>> Writer::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path path,std::string target,std::stop_token token)const{
    return std::async(std::launch::deferred,[document=std::move(document),path=std::move(path),target=std::move(target),token]{
        if(!document)throw MediaError(DiagnosticCode::HardDiskRawNullDocument);
        const auto utf8=path.extension().u8string();const std::string extension(utf8.begin(),utf8.end());
        if(!Writer{}.canWrite(*document,target,extension))throw MediaError(DiagnosticCode::HardDiskRawWrite);
        const auto& blocks=dynamic_cast<const BlockImage&>(document->representation());
        writeAtomicImage(path,[&](const ImageOutput& output,std::stop_token stop){copyBlocks(blocks,output,stop);},token);
        return std::vector<std::filesystem::path>{path};
    });
}
}
