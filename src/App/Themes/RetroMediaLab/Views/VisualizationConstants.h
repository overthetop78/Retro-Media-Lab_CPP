#pragma once
#include <array>
namespace RetroMediaLab::VisualizationConstants {
inline constexpr auto Open = "Visual.OpenImage", Empty = "Visual.NoFile", Loading = "Visual.Loading", Inspector = "Visual.Inspector";
inline constexpr auto Side = "Visual.SideLabel", Track = "Visual.TrackLabel", Sector = "Visual.SectorLabel", Size = "Visual.SizeLabel";
inline constexpr auto Revolution = "Visual.RevolutionLabel", Reset = "Common.Reset", Data = "Visual.StructureData";
inline constexpr auto Available = "Visual.SectorWithData", Missing = "Visual.SectorWithoutData", Invalid = "Visual.SectorUnreadable";
inline constexpr auto Degraded="Visual.SectorDegraded",Dead="Visual.SectorDead",State="Visual.StateLabel",LogicalBlock="Visual.LogicalBlockLabel";
inline constexpr auto SectorDataBrush="Theme.SectorDataBrush",SectorEmptyBrush="Theme.SectorEmptyBrush",SectorDegradedBrush="Theme.SectorDegradedBrush",SectorDeadBrush="Theme.SectorDeadBrush",LegendMark="●";
inline constexpr std::array SectorLabels{Available,Missing,Degraded,Dead};
inline constexpr std::array SectorBrushes{SectorDataBrush,SectorEmptyBrush,SectorDegradedBrush,SectorDeadBrush};
inline constexpr int LogicalColumn=0,SideColumn=1,TrackColumn=2,SectorColumn=3,StateColumn=6;
inline constexpr auto ZoomIn = "+", ZoomOut = "−", Percent = "%", ArgumentPrefix = "{", ArgumentSuffix = "}";
inline constexpr auto SuccessBrush = "Theme.SuccessBrush", WarningBrush = "Theme.WarningBrush", ErrorBrush = "Theme.ErrorBrush";
inline constexpr auto HexSeparator = ' ';
inline constexpr auto HexPadding = '0', LineSeparator = '\n', PercentNumberFormat = 'f';
inline constexpr auto UnexpectedKey = "Errors.UnexpectedDetails", ErrorDetail = "{detail}";
inline constexpr auto ValidKey = "Visual.CrcValid", InvalidKey = "Visual.CrcInvalid", UnavailableKey = "Visual.IntegrityUnavailable";
inline constexpr int PercentFactor = 100;
inline constexpr int Padding = 8, Gap = 6, GraphMinimumHeight = 140, HexBytesPerLine = 16, HexBase = 16, HexDigits = 2;
inline constexpr int HexPreviewBytes = 4096, InspectorColumn = 2, DetailsWeight = 1, GraphWeight = 3;
inline constexpr int RingBins = 512, AngularUnits = 16, FullCircle = 360, GraphLabelHeight = 24;
inline constexpr double HoleRadius = .24, OuterRadius = .46, ZoomStep = 1.2, InitialZoom = 1, MaximumZoom = 64;
inline constexpr double MinimumOffset = 0.0;
inline constexpr double FullTurn = 6.28318530717958647692, Half = .5, MaximumSample = 32768, AxisFraction = .45;
inline constexpr double LightFloor = .35, LightRange = .65, MinViewSize = 1, RingOverlap = .25;
inline constexpr int LineWidth = 1, SelectionWidth = 2, Alpha = 150;
inline constexpr std::array Columns{LogicalBlock,Side, Track, Sector, Size, "Visual.Integrity.Crc",State};
}
