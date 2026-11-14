#pragma once
// Technical constants; displayed text is resolved through translation catalogs.
namespace RetroMediaLab::AppConstants {
inline constexpr auto LibQt6Plugins = "/lib/Qt6/plugins";
inline constexpr auto LibThemesRetroMediaLab = "/lib/Themes/RetroMediaLab";
inline constexpr auto RetroMediaLab = "RetroMediaLab";
inline constexpr auto LibLanguages = "/lib/Languages";
inline constexpr auto PreferencesJson = "/preferences.json";
inline constexpr auto CardBrush = "CardBrush";
inline constexpr auto BorderBrush = "BorderBrush";
inline constexpr auto Console = "console";
inline constexpr auto ShellTerminalPrompt = "Shell.Terminal.Prompt";
inline constexpr auto CommandPreview = "commandPreview";
inline constexpr auto ConsoleOutput = "consoleOutput";
inline constexpr auto CopyConsole = "copyConsole";
inline constexpr auto CommonCopy = "Common.Copy";
inline constexpr auto CommonCommand = "Common.Command";
inline constexpr auto ToolRawOutput = "Tool.RawOutput";
inline constexpr auto CommonConsole = "Common.Console";
inline constexpr auto StatusReady = "Status.Ready";
inline constexpr auto TerminalBackground = "TerminalBackground";
inline constexpr auto TerminalText = "TerminalText";
inline constexpr auto TerminalCommand = "TerminalCommand";
inline constexpr auto TerminalPrompt = "TerminalPrompt";
inline constexpr auto TerminalBorder = "TerminalBorder";
inline constexpr auto Read = "read";
inline constexpr auto TabRead = "Tab.Read";
inline constexpr auto Write = "write";
inline constexpr auto TabWrite = "Tab.Write";
inline constexpr auto Convert = "convert";
inline constexpr auto TabConvert = "Tab.Convert";
inline constexpr auto Visualize = "visualize";
inline constexpr auto TabVisualize = "Tab.Visualize";
inline constexpr auto Explorer = "explorer";
inline constexpr auto TabExplorer = "Tab.Explorer";
inline constexpr auto Tools = "tools";
inline constexpr auto TabTools = "Tab.Tools";
inline constexpr auto Emulation = "emulation";
inline constexpr auto TabEmulation = "Tab.Emulation";
inline constexpr auto Version = "version";
inline constexpr auto Language = "language";
inline constexpr auto Appearance = "appearance";
inline constexpr auto Light = "light";
inline constexpr auto Dark = "dark";
inline constexpr auto System = "system";
inline constexpr auto ThemeSettings = "themeSettings";
inline constexpr auto DefaultImageDirectory = "defaultImageDirectory";
inline constexpr auto EnUS = "en-US";
inline constexpr auto General = "general";
inline constexpr auto RetroMediaLabThemeApiVersion = "RetroMediaLabThemeApiVersion";
inline constexpr auto RetroMediaLabThemeIdentifier = "RetroMediaLabThemeIdentifier";
inline constexpr auto RetroMediaLabCreateTheme = "RetroMediaLabCreateTheme";
inline constexpr auto RetroMediaLabDestroyTheme = "RetroMediaLabDestroyTheme";
inline constexpr auto RetroMediaLabCreateThemeStyle = "RetroMediaLabCreateThemeStyle";
inline constexpr auto RetroMediaLabShowTheme = "RetroMediaLabShowTheme";
inline constexpr auto RetroMediaLabSynchronizeTheme = "RetroMediaLabSynchronizeTheme";
inline constexpr auto RetroMediaLabCreateThemeConfiguration = "RetroMediaLabCreateThemeConfiguration";
inline constexpr auto RetroMediaLabDestroyThemeConfiguration = "RetroMediaLabDestroyThemeConfiguration";
inline constexpr auto RetroMediaLabConfigureTheme = "RetroMediaLabConfigureTheme";
namespace CardValues {
inline constexpr auto BorderInset = 0.5;
}
namespace ConsoleStateValues {
inline constexpr auto DefaultConsoleHeight = 190;
}
namespace PreferencesStoreValues {
inline constexpr auto LegacySchemaVersion = 1;
inline constexpr auto CurrentSchemaVersion = 2;
}
namespace StatusIndicatorValues {
inline constexpr auto IndicatorWidth = 22;
inline constexpr auto DotDiameter = 8;
inline constexpr auto LabelOffset = 10;
inline constexpr auto LeftInset = 4;
}
namespace TerminalViewValues {
inline constexpr auto Padding = 14;
inline constexpr auto CommandHeight = 30;
inline constexpr auto CopyIconSize = 22;
inline constexpr auto CopyButtonSize = 32;
inline constexpr auto BorderInset = .5;
inline constexpr auto CornerRadius = 8;
}
namespace Service {
inline constexpr auto ErrorBufferCapacity = 1024;
inline constexpr auto ApiVersion = 2;
inline constexpr auto DefaultFontSize = 14;
inline constexpr auto DefaultControlHeight = 36;
inline constexpr auto DefaultCardPadding = 18;
inline constexpr auto DefaultCardRadius = 8;
inline constexpr auto DefaultControlRadius = 4;
inline constexpr auto DefaultTabPaddingX = 22;
inline constexpr auto DefaultTabPaddingY = 11;
inline constexpr auto DefaultTabRadius = 9;
}
namespace WorkspacePagesValues {
inline constexpr auto PageCount = 7;
}
}
