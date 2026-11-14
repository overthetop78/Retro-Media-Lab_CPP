set(app_sources
  Bootstrap/Application.cpp
  State/WorkspaceState.cpp
  Windows/WindowCoordinator.cpp
  Shared/Controls/Card.cpp
  Shared/Controls/LocalizedAction.cpp
  Shared/Controls/TerminalView.cpp
  Shared/Controls/StatusIndicator.cpp
  Shared/Navigation/WorkspacePages.cpp
  Themes/ThemeService.cpp
)
add_library(RetroMediaLab.Gui OBJECT ${app_sources})
target_compile_definitions(RetroMediaLab.Gui PRIVATE RETROMEDIALAB_APP_BUILD RETROMEDIALAB_APP_VERSION="${PROJECT_VERSION}")
target_include_directories(RetroMediaLab.Gui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(RetroMediaLab.Gui PUBLIC Qt6::Widgets RetroMediaLab.Translations RetroMediaLab.Runtime RetroMediaLab.Infrastructure PRIVATE RetroMediaLab.Platform)
add_library(RetroMediaLab.App SHARED Bootstrap/EntryPoint.cpp)
target_compile_definitions(RetroMediaLab.App PRIVATE RETROMEDIALAB_APP_BUILD)
set_target_properties(RetroMediaLab.App PROPERTIES OUTPUT_NAME App PREFIX "")
target_include_directories(RetroMediaLab.App PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(RetroMediaLab.App PRIVATE RetroMediaLab.Gui PUBLIC Qt6::Widgets RetroMediaLab.Translations RetroMediaLab.Runtime RetroMediaLab.Infrastructure)
install(TARGETS RetroMediaLab.App RUNTIME DESTINATION lib LIBRARY DESTINATION lib)

add_library(RetroMediaLab.Theme.RetroMediaLab SHARED
  Themes/RetroMediaLab/RetroMediaLabConfiguration.cpp
  Themes/RetroMediaLab/ThemeEntry.cpp
  Themes/RetroMediaLab/ThemeIcons.cpp
  Themes/RetroMediaLab/Views/MainMenu.cpp
  Themes/RetroMediaLab/Views/PreferencesDialog.cpp
  Themes/RetroMediaLab/Views/HardwareSettingsPage.cpp
  Themes/RetroMediaLab/Views/KryoFluxSettingsWidget.cpp
  Themes/RetroMediaLab/Views/FloppySettingsPage.cpp
  Themes/RetroMediaLab/Views/HostToolsSettingsWidget.cpp
  Themes/RetroMediaLab/Views/MediaDeviceSettingsPage.cpp
  Themes/RetroMediaLab/Views/HardwareCapabilitiesTable.cpp
  Themes/RetroMediaLab/Views/ConversionSettingsPage.cpp
  Themes/RetroMediaLab/Views/LogSettingsPage.cpp
  Themes/RetroMediaLab/Views/ThemeSettingsPage.cpp
  Themes/RetroMediaLab/Views/ThemePreview.cpp
  Themes/RetroMediaLab/Views/WorkspaceTabs.cpp
  Themes/RetroMediaLab/Views/StatusBar.cpp
  Themes/RetroMediaLab/RetroMediaLabStyle.cpp
  Themes/RetroMediaLab/RetroMediaLabLayout.cpp
  Themes/RetroMediaLab/RetroMediaLabWindow.cpp
  Themes/RetroMediaLab/RetroMediaLabPresentation.cpp)
set_target_properties(RetroMediaLab.Theme.RetroMediaLab PROPERTIES OUTPUT_NAME RetroMediaLab PREFIX "")
target_link_libraries(RetroMediaLab.Theme.RetroMediaLab PRIVATE RetroMediaLab.App RetroMediaLab.Platform)
qt_add_resources(RetroMediaLab.Theme.RetroMediaLab ThemeData
  PREFIX "/Themes/RetroMediaLab" BASE Themes/RetroMediaLab
  FILES Themes/RetroMediaLab/icons.json Themes/RetroMediaLab/metrics.json Themes/RetroMediaLab/light.json Themes/RetroMediaLab/dark.json)
install(TARGETS RetroMediaLab.Theme.RetroMediaLab RUNTIME DESTINATION lib/Themes LIBRARY DESTINATION lib/Themes)

if(WIN32)
  enable_language(RC)
  add_executable(RetroMediaLab WIN32 ../Launcher/main.cpp ../Platform/Windows/App.rc)
  target_link_libraries(RetroMediaLab PRIVATE shell32 RetroMediaLab.Runtime)
  set_target_properties(RetroMediaLab PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  target_include_directories(RetroMediaLab PRIVATE Assets)
else()
  qt_add_executable(RetroMediaLab main.cpp)
  target_link_libraries(RetroMediaLab PRIVATE RetroMediaLab.App)
endif()
add_dependencies(RetroMediaLab RetroMediaLab.Theme.RetroMediaLab)
set_target_properties(RetroMediaLab PROPERTIES OUTPUT_NAME retromedialab)
install(TARGETS RetroMediaLab RUNTIME DESTINATION . BUNDLE DESTINATION .)
