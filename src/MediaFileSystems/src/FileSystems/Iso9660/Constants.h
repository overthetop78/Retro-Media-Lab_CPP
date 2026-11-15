#pragma once
#include <cstdint>
#include <string_view>
#include <array>
namespace RetroMediaLab::MediaFileSystems::IsoConstants {
inline constexpr int LogicalBlockSize=2048;
inline constexpr int FirstVolumeDescriptorBlock=16;
inline constexpr int VolumeDescriptorIdentifierOffset=1;
inline constexpr int VolumeDescriptorIdentifierLength=5;
inline constexpr std::string_view VolumeDescriptorIdentifier="CD001";
inline constexpr std::uint8_t PrimaryVolumeDescriptorType=1;
inline constexpr std::uint8_t SupplementaryVolumeDescriptorType=2;
inline constexpr std::uint8_t VolumeDescriptorTerminatorType=255;
inline constexpr int VolumeIdentifierOffset=40;
inline constexpr int VolumeIdentifierLength=32;
inline constexpr int VolumeSpaceSizeOffset=80;
inline constexpr int LogicalBlockSizeOffset=128;
inline constexpr int RootDirectoryRecordOffset=156;
inline constexpr int DirectoryRecordMinimumLength=34;
inline constexpr int DirectoryExtentOffset=2;
inline constexpr int DirectoryDataLengthOffset=10;
inline constexpr int DirectoryDateOffset=18;
inline constexpr int DirectoryFlagsOffset=25;
inline constexpr std::uint8_t DirectoryFlag=0x02;
inline constexpr int DirectoryFileIdentifierLengthOffset=32;
inline constexpr int DirectoryFileIdentifierOffset=33;
inline constexpr int SuspSignatureLength=2;
inline constexpr int SuspLengthOffset=2;
inline constexpr int SuspVersionOffset=3;
inline constexpr std::uint8_t SuspVersion=1;
inline constexpr std::string_view RockRidgeAlternateNameSignature="NM";
inline constexpr std::uint8_t RockRidgeAlternateNameSignatureFirstByte=std::uint8_t('N');
inline constexpr std::uint8_t RockRidgeAlternateNameSignatureSecondByte=std::uint8_t('M');
inline constexpr int RockRidgeAlternateNameMinimumLength=5;
inline constexpr int RockRidgeAlternateNameFlagsOffset=4;
inline constexpr int RockRidgeAlternateNameContentOffset=5;
inline constexpr std::uint8_t RockRidgeAlternateNameContinueFlag=0x01;
inline constexpr std::uint8_t RockRidgeAlternateNameCurrentFlag=0x02;
inline constexpr std::uint8_t RockRidgeAlternateNameParentFlag=0x04;
inline constexpr std::uint8_t SuspExtensionReferenceSignatureFirstByte=std::uint8_t('E');
inline constexpr std::uint8_t SuspExtensionReferenceSignatureSecondByte=std::uint8_t('R');
inline constexpr int SuspExtensionReferenceMinimumLength=8;
inline constexpr int SuspExtensionIdentifierLengthOffset=4;
inline constexpr int SuspExtensionIdentifierOffset=8;
inline constexpr std::string_view RockRidgeExtensionIdentifier="RRIP_1991A";
inline constexpr std::string_view RockRidgeIeeeExtensionIdentifier="IEEE_P1282";
inline constexpr int JolietEscapeOffset=88,JolietEscapeLength=3,JolietDescriptorMinimum=91,RecordingDateLength=7,RecordingDateYear=1900,RecordingDateTimezoneIndex=6,RecordingDateTimezoneMinutes=15,SuspHeaderLength=4;
inline constexpr char Null='\0',Space=' ',VersionSeparator=';',TrailingDot='.';
inline constexpr std::string_view JolietEscape1="%/@",JolietEscape2="%/C",JolietEscape3="%/E",Current=".",Parent="..",Attribute="ISO 9660";
inline constexpr int UnicodeCodeUnitLength=2,RecordingMonthOffset=1,RecordingDayOffset=2,RecordingHourOffset=3,RecordingMinuteOffset=4,RecordingSecondOffset=5;
}
