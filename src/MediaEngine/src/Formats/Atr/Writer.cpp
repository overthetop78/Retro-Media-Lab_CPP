#include "Atr.h"
#include "Constants.h"
#include "../Atari8Bit/Geometry.h"
#include "../../Primitives/Identifiers.h"
#include "../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::Atr {
namespace {
using namespace Constants;
bool matches(const MediaFileSystems::SectorImage& image,const Atari8Bit::Constants::Profile& p){return Primitives::asciiIdentifierEqualsIgnoreCase(image.geometry().formatId,p.id)&&image.blockCount()==p.sectorCount&&image.capacity()==Atari8Bit::payloadLength(p);}
Atari8Bit::Constants::Profile validateImage(const MediaFileSystems::SectorImage& image,std::string_view target){const auto profile=Atari8Bit::profile(target);if(!profile)throw MediaError(DiagnosticCode::AtrUnsupportedFormat,{std::string(target)});if(!matches(image,*profile))throw MediaError(DiagnosticCode::AtrIncompatibleImage,{image.geometry().formatId,std::to_string(image.blockCount()),std::to_string(image.capacity()),std::string(profile->id),std::to_string(profile->sectorCount),std::to_string(Atari8Bit::payloadLength(*profile))});return *profile;}
void word(std::vector<std::uint8_t>& b,std::size_t offset,std::uint32_t value){b[offset]=std::uint8_t(value);b[offset+1]=std::uint8_t(value>>BitsPerByte);}
void produce(const MediaFileSystems::SectorImage& image,const Atari8Bit::Constants::Profile& p,const ImageOutput& output,std::stop_token token){std::vector<std::uint8_t> header(HeaderSize);const auto paragraphs=Atari8Bit::payloadLength(p)/ParagraphSize;word(header,SignatureOffset,Signature);word(header,ParagraphLow,paragraphs);word(header,SectorSizeOffset,p.sectorSize);word(header,ParagraphHigh,paragraphs>>HighWordShift);output(header);for(std::uint32_t logical=0;logical<p.sectorCount;++logical){checkCancellation(token);const auto block=image.block(logical);if(!block)throw MediaError(DiagnosticCode::AtrMissingSector,{std::to_string(logical+FirstSector)});const auto expected=p.sectorSize==DoubleSize&&logical<BootCount?SingleSize:p.sectorSize;if(block->data.size()!=expected)throw MediaError(DiagnosticCode::AtrSectorSize,{std::to_string(logical+FirstSector),std::to_string(block->data.size()),std::to_string(expected)});output(block->data);}}
}
bool canWrite(const MediaFileSystems::SectorImage& image,std::string_view target){const auto p=Atari8Bit::profile(target);return p&&matches(image,*p);}
std::vector<std::uint8_t> build(const MediaFileSystems::SectorImage& image,std::string_view target,std::stop_token token){const auto p=validateImage(image,target);std::vector<std::uint8_t> bytes;produce(image,p,[&](auto part){bytes.insert(bytes.end(),part.begin(),part.end());},token);return bytes;}
void write(const MediaFileSystems::SectorImage& image,const std::filesystem::path& path,std::string_view target,std::stop_token token){const auto p=validateImage(image,target);writeAtomicImage(path,[&](const ImageOutput& output,std::stop_token stop){produce(image,p,output,stop);},token);}
}
