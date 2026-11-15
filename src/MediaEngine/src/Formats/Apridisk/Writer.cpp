#include "Internal.h"
#include "../../Models/Sectors/LinearSectorWriter.h"
#include "../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Apridisk {
bool canWrite(const MediaFileSystems::SectorImage& image){return matchesSectorGeometry(image,geometry(),false);}
std::vector<std::uint8_t> build(const MediaFileSystems::SectorImage& image,std::stop_token token){
    using namespace Constants;if(!matchesSectorGeometry(image,geometry()))throw MediaError(DiagnosticCode::ApridiskWriteGeometry);
    std::vector<std::uint8_t> bytes(HeaderSize);std::copy(Signature.begin(),Signature.end(),bytes.begin());std::vector<std::uint8_t> record(RecordHeaderSize);Internal::put<std::uint32_t>(record,0,SectorRecord);Internal::put<std::uint16_t>(record,CompressionOffset,Uncompressed);Internal::put<std::uint16_t>(record,HeaderLengthOffset,std::uint16_t(RecordHeaderSize));Internal::put<std::uint32_t>(record,PayloadLengthOffset,BlockSize);
    for(std::uint64_t logical=0;logical<BlockCount;++logical){MediaFileSystems::checkCancellation(token);const auto block=image.block(logical,token);if(!block)throw MediaError(DiagnosticCode::RawSectorMissing,{std::to_string(logical)});if(block->data.size()!=BlockSize)throw MediaError(DiagnosticCode::RawSectorSize,{std::to_string(logical),std::to_string(block->data.size()),std::to_string(BlockSize)});const auto track=logical/SectorsPerTrack;record[HeadOffset]=std::uint8_t(track%Heads);record[SectorOffset]=std::uint8_t(logical%SectorsPerTrack+1);Internal::put<std::uint16_t>(record,CylinderOffset,std::uint16_t(track/Heads));bytes.insert(bytes.end(),record.begin(),record.end());bytes.insert(bytes.end(),block->data.begin(),block->data.end());}
    return bytes;
}
void write(const MediaFileSystems::SectorImage& image,const std::filesystem::path& path,std::stop_token token){const auto bytes=build(image,token);writeAtomicImage(path,[&](const ImageOutput& out,std::stop_token stop){MediaFileSystems::checkCancellation(stop);out(bytes);},token);}
}
