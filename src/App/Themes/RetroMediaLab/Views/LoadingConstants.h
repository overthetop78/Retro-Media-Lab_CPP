#pragma once
#include <array>
namespace RetroMediaLab::LoadingConstants {
inline constexpr auto Overlay="mediaLoadingOverlay",Stage="mediaLoadingStage",Detail="mediaLoadingDetail",Bar="mediaLoadingProgress",StatusBar="mediaStatusProgress";
inline constexpr int TapeRows=2;
inline constexpr int FormatDisplayMilliseconds=650;
inline constexpr std::array<unsigned,4> TapeColors{0x6C7784,0x39444E,0xC2CAD0,0x2FA65B};
inline constexpr auto Tape="Explorer.Tape", Optical="Visual.TrackCountLabel", Blocks="Visual.SurfaceLabel";
inline constexpr auto Profile="Status.Profile",DefaultProfile="Profile.Default",ProfileObject="statusProfile",TimerObject="statusTimer",TimePattern="%1:%2:%3",Face="Visual.Side";
inline constexpr int OperationColumn=2,ProgressColumn=3,TimerColumn=4,ConsoleColumn=5;
inline constexpr int HardwareWidth=245,ProfileWidth=165,OperationWidth=125,BarHeight=30,StripRowHeight=12,StripLabelWidth=40,StripCountWidth=40,StripHeight=8,StripFont=9,SecondsPerMinute=60,SecondsPerHour=3600,MillisecondsPerSecond=1000,TimeDigits=2,DecimalBase=10,TimerInterval=1000,TimerMargin=12;
inline constexpr char LineBreak='\n';
inline constexpr char Zero='0';
inline constexpr double Half=.5,StripMargin=.5,StripRadius=1;
inline constexpr unsigned PendingRgb=0xBEC2C7,ActiveRgb=0x3E84D2,SuccessRgb=0x3CA65B,RetryRgb=0xE0972F,FailedRgb=0xCF4343,IdleRgb=0x808080,RunningRgb=0x2D7DD2,ReadingRgb=0x2E8B57;
inline constexpr auto Stop="Common.Stop",Preparing="Explorer.LoadingDisplay",Loading="Visual.Loading";
inline constexpr int Maximum=100,Width=540,Padding=20,Spacing=8,StatusWidth=140,StatusLabelWidth=100,StatusHeight=12;
}
