#pragma once
#include <array>
namespace RetroMediaLab::Infrastructure::MediaCommandConstants {
inline constexpr auto ProgressHeader=R"(^(?:Reading|Writing|Converting|Erasing)\s+c=([0-9,\-/]+):h=([0-9,\-/]+))",ProgressTrack=R"(^T(\d+)\.([01])(?::|\s|$))",ProgressRange=R"(^(\d+)(?:-(\d+)(?:/(\d+))?)?$)";
inline constexpr auto RetryToken="Retry",FailedToken="fail",ErrorToken="error";
inline constexpr int ProgressFirst=1,ProgressSecond=2,ProgressThird=3,MaximumProgressCoordinates=65536,MaximumProgressLine=8192;
inline constexpr char ProgressComma=',',ProgressNewline='\n',ProgressReturn='\r';
inline constexpr auto ReadExplorer="read-explorer",ExplorerTemporaryPattern="RetroMediaLab-explorer-XXXXXX",ExplorerCaptureName="capture";
inline constexpr auto Read = "read", Write = "write", Convert = "convert";
inline constexpr auto Raw = "--raw", HardSectors = "--hard-sectors", Reverse = "--reverse", Tg43 = "--gen-tg43";
inline constexpr auto NoClobber = "--no-clobber", PreErase = "--pre-erase", EraseEmpty = "--erase-empty", NoVerify = "--no-verify";
inline constexpr auto Retries = "--retries", SeekRetries = "--seek-retries", Revs = "--revs";
inline constexpr auto Tracks = "--tracks", OutputTracks = "--out-tracks", Pll = "--pll", Precomp = "--precomp";
inline constexpr auto FakeIndex = "--fake-index", AdjustSpeed = "--adjust-speed", Densel = "--densel", DiskDefs = "--diskdefs";
inline constexpr auto Format = "--format", Device = "--device", Drive = "--drive", Prefix = "--";
inline constexpr auto High = "H", Low = "L", Type = "type", Mfm = "mfm", Fm = "fm", Gcr = "gcr";
inline constexpr auto IntegerPattern = "^[+-]?\\d+$", ThresholdPattern = "^\\d+$";
inline constexpr auto Invalid = "Advanced.Invalid", InvalidOptions = "Tool.InvalidParameters";
inline constexpr auto NameRequired = "Read.NameRequired", TypeRequired = "Read.TypeRequired", SourceRequired = "Write.SelectSource";
inline constexpr auto MissingDefinition = "Advanced.DiskDefsMissing", MissingProvider = "Engines.MissingProvider";
inline constexpr auto NoReader = "Tool.SelectGreaseweazle", ToolMissing = "Hardware.ToolMissing";
inline constexpr auto ScpExtension = ".scp", DefaultName = "", DefaultSequence = "1";
inline constexpr auto Space = ' ', Quote = '"', LetterA = 'A', LetterZ = 'Z', DigitZero = '0';
inline constexpr auto PathSeparator = '/', OtherPathSeparator = '\\';
inline constexpr auto ReadRevsDefault = "5", ReadRetriesDefault = "5", WriteRetriesDefault = "3", TracksDefault = "c=0-79:h=0-1";
inline constexpr auto SeekRetriesDefault = "0", SpeedDefault = "300rpm", PllDefault = "period=5:phase=60", PrecompDefault = "type=mfm:40=125";
inline constexpr int AlphabetLength = 26, MaximumSequenceWidth = 16;
inline constexpr auto RevsLabel = "Read.Revolutions", RetriesLabel = "Read.Retries", TracksLabel = "Read.Tracks";
inline constexpr auto SeekRetriesLabel = "Advanced.SeekRetries", FakeIndexLabel = "Advanced.FakeIndex", HardSectorsLabel = "Advanced.HardSectors";
inline constexpr auto ReverseLabel = "Advanced.Reverse", AdjustSpeedLabel = "Advanced.AdjustSpeed", PllLabel = "Advanced.Pll";
inline constexpr auto DenselLabel = "Advanced.DensityPin", Tg43Label = "Advanced.Tg43", DiskDefsLabel = "Advanced.DiskDefs";
inline constexpr auto NoVerifyLabel = "Write.NoVerify", EraseEmptyLabel = "Write.EraseEmpty", PreEraseLabel = "Advanced.PreErase", PrecompLabel = "Advanced.Precomp";
inline constexpr auto Completed = "Operation.Succeeded", Cancelled = "Operation.Cancelled", FileExists = "Read.FileExists";
inline constexpr auto CannotDeleteCapture = "Read.CancelledFileDeleteFailed";
inline constexpr auto ProfileNameRequired = "Profile.NameRequired", DuplicateProfile = "Profile.DuplicateName";
inline constexpr auto AmbiguousWrite = "Write.Ambiguous";
inline constexpr std::array Flags{Raw,HardSectors,Reverse,Tg43,NoClobber,PreErase,EraseEmpty,NoVerify};
}
