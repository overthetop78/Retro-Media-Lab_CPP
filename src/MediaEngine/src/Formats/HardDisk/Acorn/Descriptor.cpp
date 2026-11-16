#include "Descriptor.h"
#include "Constants.h"
#include "../Adaptec/Constants.h"
#include "../Adaptec/Parameters.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
namespace RetroMediaLab::MediaEngine::AcornWinchester {
namespace {
std::int32_t sectors(std::int32_t size,bool sequential){using namespace Constants;switch(size){case SectorSize:return sequential?SmallSequentialSectors:SectorsPerTrack;case MediumSectorSize:return sequential?MediumSequentialSectors:MediumInterleavedSectors;case LargeSectorSize:return LargeSectors;default:return 0;}}
}
std::optional<HardDiskGeometry> geometry(std::span<const std::uint8_t> descriptor,std::optional<std::int64_t> length){
    using namespace Constants;if(descriptor.size()<MinimumDescriptorSize)return {};
    const auto cylinders=qFromBigEndian<std::uint16_t>(descriptor.data()+CylinderOffset);const auto heads=descriptor[HeadOffset];auto block=qFromBigEndian<std::uint32_t>(descriptor.data()+BlockSizeOffset)>>BlockSizeShift;if(!block)block=SectorSize;
    auto perTrack=sectors(std::int32_t(block),false);if(!cylinders||!heads||!perTrack)return {};const auto sequential=sectors(std::int32_t(block),true);
    if(length&&*length==std::int64_t(cylinders)*heads*sequential*block)perTrack=sequential;
    return HardDiskGeometry(cylinders,heads,perTrack,std::int32_t(block));
}
bool supportsGeometry(const HardDiskGeometry& value){using namespace Constants;const auto count=sectors(value.sectorSize(),false);return value.cylinders()<=MaximumCylinders&&value.heads()<=MaximumHeads&&count&&(value.sectorsPerTrack()==count||value.sectorsPerTrack()==sectors(value.sectorSize(),true));}
void writeGeometry(std::span<std::uint8_t> descriptor,const HardDiskGeometry& value){
    using namespace Constants;if(descriptor.size()<MinimumDescriptorSize)throw MediaError(DiagnosticCode::AcornDscSize);if(!supportsGeometry(value))throw MediaError(DiagnosticCode::AcornDscGeometry);
    descriptor[ExtentLengthOffset]=ExtentLength;qToBigEndian((std::uint32_t(value.sectorSize())<<BlockSizeShift)|descriptor[DriveListOffset],descriptor.data()+BlockSizeOffset);descriptor[DriveListOffset]=DriveListCode;qToBigEndian(std::uint16_t(value.cylinders()),descriptor.data()+CylinderOffset);descriptor[HeadOffset]=std::uint8_t(value.heads());
}
void updateGeometryIfChanged(std::span<std::uint8_t> descriptor,const HardDiskGeometry& value,std::int64_t length){
    const auto original=geometry(descriptor,length);if(original&&original->cylinders()==value.cylinders()&&original->heads()==value.heads()&&original->sectorsPerTrack()==value.sectorsPerTrack()&&original->sectorSize()==value.sectorSize())return;writeGeometry(descriptor,value);
}
bool tryWritePhysicalParameters(std::span<const std::uint8_t> descriptor,std::int64_t length,std::span<std::uint8_t> flags){
    if(flags.size()<Adaptec::Constants::ParameterFlagCount)throw MediaError(DiagnosticCode::AdaptecParameterSize);const auto value=geometry(descriptor,length);
    if(!value||value->cylinders()>Adaptec::Constants::MaximumCylinders||value->heads()>Adaptec::Constants::MaximumHeads)return false;
    const auto current=qFromBigEndian<std::uint16_t>(descriptor.data()+Constants::ReducedWriteCurrentOffset);const auto step=descriptor[Constants::StepRateOffset];if(current>Adaptec::Constants::MaximumReducedCurrentCylinder||step>Adaptec::Constants::MaximumStepCode)return false;
    Adaptec::writeInitialParameters(flags,value->sectorSize(),std::int32_t(value->cylinders()),value->heads(),current,descriptor[Constants::LandingZoneOffset],step);return true;
}
}
