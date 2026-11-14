#pragma once
// Technical constants; displayed text is resolved through translation catalogs.
namespace RetroMediaLab::ThemeConstants {
inline constexpr auto Logs = "logs";
inline constexpr auto LogsTitle = "Options.Logs";
inline constexpr auto LogsIcon = "logs";
namespace LogPage {
inline constexpr auto Page = "logSettingsPage";
inline constexpr auto Scroll = "logOptionsScroll";
inline constexpr auto EnabledPrefix = "logsEnabled_";
inline constexpr auto MaximumPrefix = "logsMaximum_";
inline constexpr auto ArchivesPrefix = "logsArchives_";
inline constexpr auto Directory = "logsDirectory";
inline constexpr auto OpenFolder = "openLogsDirectory";
inline constexpr auto Description = "Options.LogsDescription";
inline constexpr auto Enable = "Options.EnableLogs";
inline constexpr auto Maximum = "Options.LogMaximumSize";
inline constexpr auto KiB = "Options.Kilobytes";
inline constexpr auto Archives = "Options.KeepLogArchives";
inline constexpr auto UnlimitedHint = "Options.LogUnlimitedHint";
inline constexpr auto OpenFolderLabel = "Options.OpenLogsFolder";
inline constexpr auto OpenFolderError = "Logs.OpenFolderError";
inline constexpr auto ErrorTitle = "Errors.DialogTitle";
inline constexpr auto Detail = "{detail}";
inline constexpr auto Digits = "[0-9]*";
inline constexpr int ActionWidth = 190;
inline constexpr int ToggleWidth = 210;
inline constexpr int SizeWidth = 280;
inline constexpr int InputWidth = 72;
inline constexpr int Gap = 8;
inline constexpr int UnitGap = 7;
inline constexpr int RowGap = 6;
inline constexpr int DescriptionGap = 10;
inline constexpr int Index = 2;
}
inline constexpr auto ChooseTheme = "chooseTheme";
inline constexpr auto ThemesChoose = "Themes.Choose";
inline constexpr auto ChooseParameter = "{choose}";
inline constexpr auto ThemesRetroMediaLabMetricsJson = ":/Themes/RetroMediaLab/metrics.json";
inline constexpr auto FontSize = "fontSize";
inline constexpr auto ControlHeight = "controlHeight";
inline constexpr auto CardPadding = "cardPadding";
inline constexpr auto CardRadius = "cardRadius";
inline constexpr auto ControlRadius = "controlRadius";
inline constexpr auto TabPaddingX = "tabPaddingX";
inline constexpr auto TabPaddingY = "tabPaddingY";
inline constexpr auto TabRadius = "tabRadius";
inline constexpr auto SegoeUI = "Segoe UI";
inline constexpr auto ThemesRetroMediaLabDarkJson = ":/Themes/RetroMediaLab/dark.json";
inline constexpr auto ThemesRetroMediaLabLightJson = ":/Themes/RetroMediaLab/light.json";
inline constexpr auto WindowBrush = "WindowBrush";
inline constexpr auto CardBrush = "CardBrush";
inline constexpr auto ControlBrush = "ControlBrush";
inline constexpr auto TextBrush = "TextBrush";
inline constexpr auto BorderBrush = "BorderBrush";
inline constexpr auto AccentBrush = "AccentBrush";
inline constexpr auto MutedTextBrush = "MutedTextBrush";
inline constexpr auto SelectedBrush = "SelectedBrush";
inline constexpr auto HoverBrush = "HoverBrush";
inline constexpr auto StatusBrush = "StatusBrush";
inline constexpr auto TerminalBackground = "TerminalBackground";
inline constexpr auto TerminalBorder = "TerminalBorder";
inline constexpr auto TerminalText = "TerminalText";
inline constexpr auto TerminalCommand = "TerminalCommand";
inline constexpr auto TerminalPrompt = "TerminalPrompt";
inline constexpr auto CaptionBrush = "CaptionBrush";
inline constexpr auto Appearance = "appearance";
inline constexpr auto Light = "light";
inline constexpr auto System = "system";
inline constexpr auto Dark = "dark";
inline constexpr auto ExplorerSelectionBrush = "ExplorerSelectionBrush";
inline constexpr auto WorkspaceSplitter = "workspaceSplitter";
inline constexpr auto Copy = "copy";
inline constexpr auto Fusion = "Fusion";
inline constexpr auto RetroMediaLabWindow = "RetroMediaLabWindow";
inline constexpr auto AppTitle = "App.Title";
inline constexpr auto AssetsAppIconIco = "/Assets/app-icon.ico";
inline constexpr auto ErrorBufferFormat = "%s";
inline constexpr auto ThemesRetroMediaLab = ":/Themes/RetroMediaLab";
inline constexpr auto RetroMediaLab = "RetroMediaLab";
inline constexpr auto ThemesRetroMediaLabIconsJson = ":/Themes/RetroMediaLab/icons.json";
inline constexpr auto CanvasSize = "canvasSize";
inline constexpr auto DevicePixelRatio = "devicePixelRatio";
inline constexpr auto StrokeWidth = "strokeWidth";
inline constexpr auto Paths = "paths";
inline constexpr auto Move = "move";
inline constexpr auto Line = "line";
inline constexpr auto Quad = "quad";
inline constexpr auto Cubic = "cubic";
inline constexpr auto Rect = "rect";
inline constexpr auto Ellipse = "ellipse";
inline constexpr auto Close = "close";
inline constexpr auto MainMenu = "mainMenu";
inline constexpr auto MenuOptions = "Menu.Options";
inline constexpr auto MenuPreferences = "Menu.Preferences";
inline constexpr auto UpdatesTitle = "Updates.Title";
inline constexpr auto LogsHistory = "Logs.History";
inline constexpr auto MenuDiagnostics = "Menu.Diagnostics";
inline constexpr auto MenuInformation = "Menu.Information";
inline constexpr auto MenuBandwidth = "Menu.Bandwidth";
inline constexpr auto MenuRpm = "Menu.Rpm";
inline constexpr auto MenuSeek = "Menu.Seek";
inline constexpr auto MenuAlign = "Menu.Align";
inline constexpr auto MenuHardware = "Menu.Hardware";
inline constexpr auto MenuPins = "Menu.Pins";
inline constexpr auto MenuReset = "Menu.Reset";
inline constexpr auto MenuDelays = "Menu.Delays";
inline constexpr auto MenuFirmware = "Menu.Firmware";
inline constexpr auto MenuEmulation = "Menu.Emulation";
inline constexpr auto MenuEmulationPreferences = "Menu.EmulationPreferences";
inline constexpr auto MenuHelp = "Menu.Help";
inline constexpr auto MenuDocumentation = "Menu.Documentation";
inline constexpr auto MenuAbout = "Menu.About";
inline constexpr auto PreferencesDialog = "preferencesDialog";
inline constexpr auto PreferencesSections = "preferencesSections";
inline constexpr auto DestinationCard = "destinationCard";
inline constexpr auto DefaultImageDirectory = "defaultImageDirectory";
inline constexpr auto BrowseImageDirectory = "browseImageDirectory";
inline constexpr auto LanguageCard = "languageCard";
inline constexpr auto PreferencesLanguage = "preferencesLanguage";
inline constexpr auto ClosePreferences = "closePreferences";
inline constexpr auto Themes = "themes";
inline constexpr auto General = "general";
inline constexpr auto PreferencesSelectDirectory = "Preferences.SelectDirectory";
inline constexpr auto PreferencesTitle = "Preferences.Title";
inline constexpr auto PreferencesGeneral = "Preferences.General";
inline constexpr auto PreferencesThemes = "Preferences.Themes";
inline constexpr auto WindowLanguage = "Window.Language";
inline constexpr auto PreferencesDefaultImageDirectory = "Preferences.DefaultImageDirectory";
inline constexpr auto PreferencesBrowse = "Preferences.Browse";
inline constexpr auto PreferencesClose = "Preferences.Close";
inline constexpr auto Preferences = "preferences";
inline constexpr auto StatusBar = "statusBar";
inline constexpr auto ReadyStatus = "readyStatus";
inline constexpr auto ToggleConsole = "toggleConsole";
inline constexpr auto HardwareNotConfigured = "Hardware.NotConfigured";
inline constexpr auto HardwareSelector = "statusHardwareSelector";
inline constexpr auto HardwareIndicator = "statusHardwareIndicator";
inline constexpr auto HardwareDriveChoice = "Hardware.DriveChoice";
inline constexpr auto HardwareDisconnected = "Hardware.Disconnected";
inline constexpr auto HardwareDetailSeparator = " · ";
inline constexpr auto HardwareDisconnectedFormat = "%1 (%2)";
inline constexpr auto HardwareField0 = "{0}";
inline constexpr auto HardwareField1 = "{1}";
inline constexpr auto HardwareField2 = "{2}";
inline constexpr auto HardwareField3 = "{3}";
inline constexpr auto StatusReadyShort = "Status.ReadyShort";
inline constexpr auto CommonConsole = "Common.Console";
inline constexpr auto Console = "console";
inline constexpr auto ThemePreview = "themePreview";
inline constexpr auto ThemeSettingsPage = "themeSettingsPage";
inline constexpr auto AvailableThemesCard = "availableThemesCard";
inline constexpr auto AvailableThemes = "availableThemes";
inline constexpr auto ThemeName = "Theme.Name";
inline constexpr auto AppearanceSystem = "appearanceSystem";
inline constexpr auto AppearanceLight = "appearanceLight";
inline constexpr auto AppearanceDark = "appearanceDark";
inline constexpr auto ActiveTheme = "activeTheme";
inline constexpr auto ThemesAvailable = "Themes.Available";
inline constexpr auto ThemesSystem = "Themes.System";
inline constexpr auto WindowLight = "Window.Light";
inline constexpr auto WindowDark = "Window.Dark";
inline constexpr auto ThemesVariants = "Themes.Variants";
inline constexpr auto System2 = "{system}";
inline constexpr auto Light2 = "{light}";
inline constexpr auto Dark2 = "{dark}";
inline constexpr auto Space = " ";
inline constexpr auto ThemesDescription = "Themes.Description";
inline constexpr auto WindowTheme = "Window.Theme";
inline constexpr auto ThemesPersisted = "Themes.Persisted";
inline constexpr auto ThemesPreview = "Themes.Preview";
inline constexpr auto ThemesActive = "Themes.Active";
inline constexpr auto Theme = "{theme}";
inline constexpr auto Appearance2 = "{appearance}";
inline constexpr auto WorkspaceTabs = "workspaceTabs";
namespace Menu {
inline constexpr auto HorizontalInset = 3;
inline constexpr auto VerticalInset = 2;
inline constexpr auto CombinedVerticalInset = VerticalInset * 2;
}
namespace PreferencesLayout {
inline constexpr auto OuterPadding = 16;
inline constexpr auto TabIconExtent = 17;
inline constexpr auto CardPadding = 18;
inline constexpr auto CardSpacing = 12;
inline constexpr auto ActionMinimumWidth = 120;
inline constexpr auto DirectoryColumnWeight = 3;
inline constexpr auto LanguageColumnWeight = 2;
inline constexpr auto ScreenInset = 48;
inline constexpr auto MinimumWidth = 640;
inline constexpr auto MinimumHeight = 400;
inline constexpr auto DefaultWidth = 1100;
}
namespace Configuration {
inline constexpr auto MinimumMetric = 1;
inline constexpr auto MaximumMetric = 256;
inline constexpr auto DarkLightnessThreshold = .5;
}
namespace Layout {
inline constexpr auto ConsoleToggleIconSize = 16;
inline constexpr auto PagePadding = 10;
inline constexpr auto ContentSpacing = 8;
inline constexpr auto OuterSpacing = 4;
inline constexpr auto MinimumContentHeight = 110;
inline constexpr auto MinimumConsoleHeight = 100;
}
namespace Style {
inline constexpr auto BorderInset = .5;
inline constexpr auto TabFrameRadius = 9;
inline constexpr auto ButtonBorderInset = 0.5;
inline constexpr auto TabIconExtent = 17;
inline constexpr auto IconTextGap = 8;
inline constexpr auto TabUnderlineBottomInset = 2;
inline constexpr auto TabUnderlineThickness = 3;
inline constexpr auto MenuBarHorizontalPadding = 28;
inline constexpr auto MenuBarVerticalPadding = 18;
inline constexpr auto MenuItemVerticalPadding = 14;
inline constexpr auto TabUnderlineInset = 12;
inline constexpr auto TabRightInset = 3.5;
inline constexpr auto TabBottomExtension = 5;
}
namespace Window {
inline constexpr auto ScreenInset = 48;
inline constexpr auto MinimumWidth = 800;
inline constexpr auto MinimumHeight = 480;
inline constexpr auto DefaultWidth = 1360;
inline constexpr auto DefaultHeight = 820;
}
namespace StatusBarLayout {
inline constexpr auto HardwareMaximumWidth = 480;
inline constexpr auto BarHeight = 30;
inline constexpr auto HorizontalInset = 8;
inline constexpr auto VerticalInset = 2;
inline constexpr auto CombinedVerticalInset = VerticalInset * 2;
inline constexpr auto ToggleExtent = 22;
inline constexpr auto ToggleIconSize = 20;
}
namespace Entry {
inline constexpr auto ApiVersion = 2;
}
namespace Icons {
inline constexpr auto OperationIndex = 0;
inline constexpr auto FirstCoordinate = 1;
inline constexpr auto ThirdCoordinate = 3;
inline constexpr auto FifthCoordinate = 5;
inline constexpr auto CloseCommandLength = 1;
inline constexpr auto MaximumCanvasSize = 256;
inline constexpr auto PairCommandLength = 3;
inline constexpr auto SecondCoordinate = 2;
inline constexpr auto QuadCommandLength = 5;
inline constexpr auto FourthCoordinate = 4;
inline constexpr auto CubicCommandLength = 7;
inline constexpr auto SixthCoordinate = 6;
}
namespace ThemeSettings {
inline constexpr auto PreviewMinimumHeight = 150;
inline constexpr auto PreviewMaximumHeight = 260;
inline constexpr auto BorderInset = .5;
inline constexpr auto PreviewInset = 24;
inline constexpr auto CenterDivisor = 2;
inline constexpr auto PagePadding = 18;
inline constexpr auto ColumnSpacing = 16;
inline constexpr auto ListWidth = 230;
inline constexpr auto DetailSpacing = 12;
inline constexpr auto ListEntryHeight = 82;
inline constexpr auto ListEntryHorizontalInset = 10;
inline constexpr auto ListEntryVerticalInset = 8;
inline constexpr auto HeadingFontSize = 20;
}
namespace Tabs {
inline constexpr auto IconExtent = 17;
}
namespace CapabilityTable {
inline constexpr auto InformationMark = "—";
inline constexpr int Columns = 4;
inline constexpr int StatusColumn = 1;
inline constexpr int SourceColumn = 2;
inline constexpr int DetailColumn = 3;
inline constexpr int FrameMultiplier = 2;
inline constexpr auto ObjectName = "hardwareCapabilitiesTable";
inline constexpr auto Pass = "Hardware.Pass";
inline constexpr auto Fail = "Hardware.Fail";
inline constexpr auto Capability = "Hardware.Capability";
inline constexpr auto Status = "Hardware.CapabilityStatus";
inline constexpr auto Source = "Hardware.CapabilitySource";
inline constexpr auto Detail = "Hardware.CapabilityDetails";
}
namespace EnginesPage {
inline constexpr int Index = 5;
inline constexpr int Padding = 12;
inline constexpr int CardPadding = 16;
inline constexpr int Gap = 12;
inline constexpr int ComboMinimumWidth = 300;
inline constexpr auto Page = "engineSettingsPage";
inline constexpr auto Tabs = "engineBackendTabs";
inline constexpr auto GreaseweazlePage = "greaseweazleEngineSettings";
inline constexpr auto KryoFluxPage = "kryofluxEngineSettings";
inline constexpr auto ChoicePrefix = "engineChoice_";
inline constexpr auto Icon = "engines";
}
namespace ProfilesPage {
inline constexpr int CategoryCount = 3;
inline constexpr int Index = 6;
inline constexpr int Padding = 18;
inline constexpr int CardPadding = 14;
inline constexpr int Gap = 12;
inline constexpr auto Page = "profileSettingsPage";
inline constexpr auto Section = "profiles";
inline constexpr auto Title = "Profiles.Title";
inline constexpr auto Icon = "profiles";
inline constexpr const char* ListNames[] = {"readProfilesList", "writeProfilesList", "conversionProfilesList"};
inline constexpr const char* LabelKeys[] = {"Profiles.Read", "Profiles.Write", "Profiles.Conversion"};
}

namespace Density {
inline constexpr auto CompactFactor = 0.85;
inline constexpr auto MinimumScreenFactor = 0.75;
inline constexpr int ReferenceWidth = 1600;
inline constexpr int ReferenceHeight = 900;
}

}
