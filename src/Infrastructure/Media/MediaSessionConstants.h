#pragma once
namespace RetroMediaLab::Infrastructure::MediaSessionConstants {
inline constexpr int PollMilliseconds = 40;
inline constexpr int TrackPresentationMilliseconds=30, ScpTrackPresentationMilliseconds=75, SectorTrackPresentationMilliseconds=100, TapePresentationMilliseconds=100;
inline constexpr int LinearPresentationBudgetMilliseconds=3000, MinimumLinearPresentationMilliseconds=1;
inline constexpr auto UnexpectedError = "Errors.UnexpectedDetails";
inline constexpr auto MediaDomain = "MediaEngine";
inline constexpr double MaximumZoom = 64;
}
