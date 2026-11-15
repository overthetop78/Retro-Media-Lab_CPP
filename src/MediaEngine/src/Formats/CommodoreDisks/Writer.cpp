#include "Images.h"
#include "Constants.h"
#include "../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaFileSystems/CommodoreGeometry.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QString>
#include "../../Models/Sectors/LinearSectorWriter.h"
#include <RetroMediaLab/MediaFileSystems/CommodoreGeometryConstants.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::CommodoreDisks {
namespace {
MediaFileSystems::SectorGeometry d81Geometry() {
    namespace C=MediaFileSystems::Commodore::Constants;
    return {std::string(C::Format1581),C::SectorSize,C::Cylinders1581,C::Heads1581,C::BlocksPerTrack1581};
}
}
void writeD64D71(const MediaFileSystems::SectorImage& image,const std::filesystem::path& path,ErrorMapMode mode,std::stop_token token){
    namespace C=MediaFileSystems::Commodore::Constants;
    const auto& geometry=image.geometry();
    if(std::find(C::TrackCounts.begin(),C::TrackCounts.end(),geometry.cylinders)==C::TrackCounts.end())throw MediaError(DiagnosticCode::CommodoreWriteGeometry,{geometry.formatId,std::to_string(geometry.cylinders),std::to_string(geometry.heads),std::to_string(image.blockCount())});
    const auto blocks=std::uint64_t(MediaFileSystems::Commodore::blocksPerSide1541(std::int32_t(geometry.cylinders)))*geometry.heads;
    const auto format=QString::fromStdString(geometry.formatId);
    const auto validFormat=(format.compare(QString::fromUtf8(C::Format1541),Qt::CaseInsensitive)==0&&geometry.heads==1)||
        (format.compare(QString::fromUtf8(C::Format1571),Qt::CaseInsensitive)==0&&geometry.heads==C::TwoSides);
    if(!validFormat||geometry.blockSize!=C::SectorSize||image.blockCount()!=blocks)
        throw MediaError(DiagnosticCode::CommodoreWriteGeometry,{geometry.formatId,std::to_string(geometry.cylinders),std::to_string(geometry.heads),std::to_string(image.blockCount())});
    writeAtomicImage(path,[&](const ImageOutput& output,std::stop_token current){
        for(std::uint64_t logical=0;logical<image.blockCount();++logical){
            checkCancellation(current);const auto block=image.block(logical,current);
            if(!block)throw MediaError(DiagnosticCode::CommodoreWriteMissing,{std::to_string(logical)});
            if(block->data.size()!=C::SectorSize)throw MediaError(DiagnosticCode::CommodoreWriteSize,{std::to_string(logical),std::to_string(block->data.size()),std::to_string(C::SectorSize)});
            output(block->data);
        }
        if(mode==ErrorMapMode::Preserve){
            for(std::uint64_t logical=0;logical<image.blockCount();++logical){
                checkCancellation(current);const auto block=image.block(logical,current);
                if(!block)throw MediaError(DiagnosticCode::CommodoreWriteMissing,{std::to_string(logical)});
                if(!block->diagnosticCode)throw MediaError(DiagnosticCode::CommodoreWriteDiagnostic,{std::to_string(logical)});
                const auto diagnostic=*block->diagnosticCode;output(std::span<const std::uint8_t>(&diagnostic,1));
            }
        }
    },token);
}
bool canWriteD81(const MediaFileSystems::SectorImage& image) {return matchesSectorGeometry(image,d81Geometry());}
void writeD81(const MediaFileSystems::SectorImage& image,const std::filesystem::path& path,std::stop_token token) {writeLinearSectorImage(image,path,d81Geometry(),true,token);}
}
