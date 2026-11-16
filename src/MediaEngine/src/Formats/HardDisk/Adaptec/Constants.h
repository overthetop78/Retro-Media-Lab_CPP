#pragma once
#include <cstdint>
#include <cstddef>
namespace RetroMediaLab::MediaEngine::Adaptec::Constants {
inline constexpr std::size_t ParameterFlagCount=9,ParameterCylinderOffset=2,ParameterHeadUpperOffset=4,ParameterDerivedOffset=6,ParameterLandingOffset=8;
inline constexpr int ParameterWordShift=7,ParameterAuxiliaryShift=5,ParameterHeadUpperShift=2,MaximumCylinders=2048,MaximumHeads=16,MinimumBlockSize=256,MediumBlockSize=512,MaximumBlockSize=1024,MaximumReducedCurrentCylinder=2047;
inline constexpr std::uint8_t DefectCountMask=0x7f,ParameterWordHighMask=0x1f,ParameterAuxiliaryMask=3,LandingBeforeTrackZeroMask=0x80,LandingSignFlag=0x40,MaximumStepCode=2;
}
