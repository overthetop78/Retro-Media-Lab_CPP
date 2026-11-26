#pragma once
namespace RetroMediaLab::Infrastructure::OpeningProgressConstants {
inline constexpr double Zero=0,Unit=1;
inline constexpr double Initial=12,ProbeWeight=13,DecodeStart=25,DecodeWeight=55,CandidateStart=80,CandidateWeight=8,Maximum=100;
inline constexpr auto TapeReading="Explorer.LoadingTapeReading",TapeRecord="Explorer.LoadingTapeRecord";
inline constexpr auto Read="Tab.Read",Write="Tab.Write",Convert="Tab.Convert",TrackProgress="Status.TrackProgress",Display="Explorer.LoadingDisplay";
inline constexpr auto Recognition="Explorer.LoadingRecognition",Reading="Explorer.LoadingMedia",FileSystem="Explorer.LoadingFileSystem",Contents="Explorer.LoadingContents",TapeDecode="Explorer.LoadingTapeDecoding",Files="Explorer.LoadingFileRecognition";
inline constexpr auto Testing="Explorer.TestingFormat",Recognized="Explorer.TestedFormatRecognized",Rejected="Explorer.TestedFormatRejected",ReadingFormat="Explorer.ReadingFormat",Track="Visual.AnalysingTrack",Revolution="Visual.AnalysingRevolution";
}
