#include "Parameters.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Adaptec {
void writeInitialParameters(std::span<std::uint8_t> flags,std::int32_t size,std::int32_t cylinders,std::int32_t heads,std::uint16_t current,std::uint8_t landing,std::uint8_t step){
    using namespace Constants;
    if(flags.size()<ParameterFlagCount)throw MediaError(DiagnosticCode::AdaptecParameterSize);
    if(cylinders<1||cylinders>MaximumCylinders||heads<1||heads>MaximumHeads||(size!=MinimumBlockSize&&size!=MediumBlockSize&&size!=MaximumBlockSize)||current>MaximumReducedCurrentCylinder||step>MaximumStepCode)throw MediaError(DiagnosticCode::AdaptecParameterValue);
    std::fill_n(flags.begin(),ParameterFlagCount,0);flags[0]=std::uint8_t(size&DefectCountMask);flags[1]=std::uint8_t(((size>>ParameterWordShift)&ParameterWordHighMask)|(step<<ParameterAuxiliaryShift));
    const auto lastCylinder=cylinders-1,lastHead=heads-1;flags[ParameterCylinderOffset]=std::uint8_t(lastCylinder&DefectCountMask);flags[ParameterCylinderOffset+1]=std::uint8_t((lastCylinder>>ParameterWordShift)|((lastHead&ParameterAuxiliaryMask)<<ParameterAuxiliaryShift));
    flags[ParameterHeadUpperOffset]=std::uint8_t(current&DefectCountMask);flags[ParameterHeadUpperOffset+1]=std::uint8_t((current>>ParameterWordShift)|((lastHead>>ParameterHeadUpperShift)<<ParameterAuxiliaryShift));
    flags[ParameterDerivedOffset]=flags[ParameterHeadUpperOffset];flags[ParameterDerivedOffset+1]=std::uint8_t((flags[ParameterHeadUpperOffset+1]&ParameterWordHighMask)|((landing&LandingBeforeTrackZeroMask)?LandingSignFlag:0));flags[ParameterLandingOffset]=std::uint8_t(landing&DefectCountMask);
}
}
