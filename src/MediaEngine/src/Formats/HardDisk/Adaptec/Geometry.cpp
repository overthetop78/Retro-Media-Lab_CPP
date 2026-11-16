#include "Geometry.h"
#include "Constants.h"
#include <array>
namespace RetroMediaLab::MediaEngine::Adaptec {
std::optional<Geometry> readGeometry(std::span<const Field> fields){
    using namespace Constants;if(fields.size()<ParameterFlagCount||fields.size()>MaximumSectors)return {};std::optional<std::size_t> first;std::uint64_t addresses{};
    for(std::size_t i=0;i<fields.size();++i){const auto& field=fields[i];const auto number=field.sector.address.number;
        if(field.id.size()!=IdLength||field.sector.headerIntegrityValid!=true||(field.id[AddressOffset]&DefectiveAddressByteMask)||number<0||std::uint64_t(number)>=fields.size())return {};
        const auto mask=std::uint64_t(1)<<number;if(addresses&mask)return {};addresses|=mask;if(bool(field.id[FlagsOffset]&LastSectorFlag)!=(std::uint64_t(number)==fields.size()-1))return {};if(number==0)first=i;
    }
    if(!first)return {};std::array<std::uint8_t,ParameterFlagCount> flags{};for(std::size_t i=0;i<flags.size();++i)flags[i]=fields[(*first+i)%fields.size()].id[FlagsOffset]&DefectCountMask;
    const auto word=[&](std::size_t offset){return flags[offset]|((flags[offset+1]&ParameterWordHighMask)<<ParameterWordShift);};
    const auto size=word(0),cylinders=word(ParameterCylinderOffset)+1,heads=(flags[ParameterCylinderOffset+1]>>ParameterAuxiliaryShift)|((flags[ParameterHeadUpperOffset+1]>>ParameterAuxiliaryShift)<<ParameterHeadUpperShift);
    const auto count=std::int32_t(fields.size());const auto validCount=size==MinimumBlockSize?(count==SmallSequentialSectors||count==SmallInterleavedSectors):size==MediumBlockSize?(count==MediumSequentialSectors||count==MediumInterleavedSectors):size==MaximumBlockSize&&count==LargeSectors;
    if(!validCount||cylinders>MaximumCylinders||heads+1>MaximumHeads)return {};return Geometry{size,cylinders,heads+1,count};
}
}
