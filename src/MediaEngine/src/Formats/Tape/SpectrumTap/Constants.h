#pragma once
#include <string_view>
#include <array>
#include <cstdint>
#include "../Tzx/Constants.h"
namespace RetroMediaLab::MediaEngine::SpectrumTap::Constants {
inline constexpr int LengthFieldSize=2;
inline constexpr std::uint8_t HeaderFlag=0x00;
inline constexpr std::uint8_t DataFlag=0xff;
inline constexpr int HeaderBlockLength=19;
inline constexpr std::uint16_t PilotPulseTStates=Tzx::Constants::StandardPilotPulseTStates;
inline constexpr std::uint16_t SyncFirstPulseTStates=Tzx::Constants::StandardSyncFirstPulseTStates;
inline constexpr std::uint16_t SyncSecondPulseTStates=Tzx::Constants::StandardSyncSecondPulseTStates;
inline constexpr std::uint16_t ZeroPulseTStates=Tzx::Constants::StandardZeroPulseTStates;
inline constexpr std::uint16_t OnePulseTStates=Tzx::Constants::StandardOnePulseTStates;
inline constexpr std::uint16_t HeaderPilotPulseCount=Tzx::Constants::StandardHeaderPilotPulseCount;
inline constexpr std::uint16_t DataPilotPulseCount=Tzx::Constants::StandardDataPilotPulseCount;
inline constexpr int MinimumDetectedPilotPulseCount=256;
inline constexpr double PulseToleranceRatio=0.35;
inline constexpr int PcmBitsPerSample=16;
inline constexpr int PcmChannelCount=1;
inline constexpr std::int16_t PcmAmplitude=24576;
inline constexpr std::string_view BlockMetadataKey="spectrumTapBlock";
inline constexpr std::string_view DecoderId="spectrum-tape";
inline constexpr std::string_view EncoderId="spectrum-tape";
inline constexpr std::string_view FormatId="tape.spectrum-tap",WriterId="tape-spectrum-tap",Extension=".tap",FlagKey="flag",ChecksumKey="checksumValid",HeaderKey="header",TimingKey="timing",NoTiming="not-stored",TrueValue="True",FalseValue="False";
inline constexpr int MinimumBlockLength=2,MaximumBlockLength=65535,HexRadix=16,HexWidth=2;
inline constexpr char DigitZero='0';
}
