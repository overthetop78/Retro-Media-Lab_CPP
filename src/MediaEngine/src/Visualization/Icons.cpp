#include <RetroMediaLab/MediaEngine/DiskClassification.h>
#include <RetroMediaLab/MediaEngine/MediaIconIds.h>
#include "IconConstants.h"
#include "../Primitives/Identifiers.h"
#include "../Primitives/Path.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace {
template<class Range>bool contains(const Range& values,std::string_view suffix){return std::any_of(std::begin(values),std::end(values),[&](auto value){return Primitives::ordinalIdentifierEqualsIgnoreCase(value,suffix);});}
}
std::string_view selectMediaIcon(std::optional<MediaKind> kind,const std::filesystem::path& path,const DiskFormat* format){
 namespace C=Visualization::IconConstants;namespace I=MediaIconIds;const auto suffix=Primitives::pathExtension(path);
 if(kind)switch(*kind){
 case MediaKind::Floppy:
  if(format)switch(format->formFactor){case FloppyFormFactor::ThreeInch:return I::FloppyThreeInch;case FloppyFormFactor::ThreeAndHalfInch:if(format->density==FloppyDensity::ExtendedDensity)return I::FloppyThreeAndHalfExtended;if(format->density==FloppyDensity::HighDensity)return I::FloppyThreeAndHalfHigh;return I::FloppyThreeAndHalfDouble;case FloppyFormFactor::FiveAndQuarterInch:return I::FloppyFiveAndQuarter;case FloppyFormFactor::EightInch:return I::FloppyEightInch;default:break;}return I::Floppy;
 case MediaKind::HardDisk:return I::HardDisk;
 case MediaKind::Optical:return I::Optical;
 case MediaKind::Tape:return contains(C::DataTapeExtensions,suffix)?I::TapeData:I::Tape;
 case MediaKind::Cartridge:return I::Cartridge;
 default:break;
 }
 if(contains(C::CartridgeExtensions,suffix))return I::Cartridge;if(contains(C::OpticalExtensions,suffix))return I::Optical;return I::File;
}
}
