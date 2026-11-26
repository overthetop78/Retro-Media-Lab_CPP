set(app_sources
  Bootstrap/Application.cpp
  State/WorkspaceState.cpp
  Windows/WindowCoordinator.cpp
  Shared/Controls/HexGrid.cpp
  Shared/Controls/Card.cpp
  Shared/Controls/LocalizedAction.cpp
  Shared/Controls/TerminalView.cpp
  Shared/Controls/StatusIndicator.cpp
  Shared/Navigation/WorkspacePages.cpp
  Shared/Localization/MediaText.cpp
  Shared/Presentation/ThemeService.cpp
  Shared/Controls/FlowLayout.cpp
  Shared/Controls/ContentScrollArea.cpp
  Shared/Emulation/EmulationHostKeyboard.cpp
  Shared/Emulation/EmulationWorkspace.cpp
  Shared/Emulation/EmulationControllerFeedback.cpp
  Shared/Controls/ControllerArtwork.cpp
  Shared/Controls/ControllerVisualProfiles.cpp
  Shared/Controls/ControllerAnalogProfiles.cpp
  Shared/Controls/ControllerOutputs.cpp
  Shared/Controls/MachineInputBindings.cpp
)
add_library(RetroMediaLab.Gui OBJECT ${app_sources})
target_compile_definitions(RetroMediaLab.Gui PRIVATE RETROMEDIALAB_APP_BUILD RETROMEDIALAB_APP_VERSION="${PROJECT_VERSION}")
target_include_directories(RetroMediaLab.Gui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(RetroMediaLab.Gui PUBLIC Qt6::Widgets RetroMediaLab.Translations RetroMediaLab.Runtime RetroMediaLab.Infrastructure PRIVATE RetroMediaLab.Platform RetroMediaLab.Emulation RetroMediaLab.VideoPresentation)
add_library(RetroMediaLab.App SHARED Bootstrap/EntryPoint.cpp)
target_compile_definitions(RetroMediaLab.App PRIVATE RETROMEDIALAB_APP_BUILD)
set_target_properties(RetroMediaLab.App PROPERTIES OUTPUT_NAME App PREFIX "")
target_include_directories(RetroMediaLab.App PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(RetroMediaLab.App PRIVATE RetroMediaLab.Gui PUBLIC Qt6::Widgets RetroMediaLab.Translations RetroMediaLab.Runtime RetroMediaLab.Infrastructure)
install(TARGETS RetroMediaLab.App RUNTIME DESTINATION lib LIBRARY DESTINATION lib)

add_library(RetroMediaLab.Theme.RetroMediaLab SHARED
  Themes/RetroMediaLab/DisplayDensity.cpp
  Themes/RetroMediaLab/RetroMediaLabConfiguration.cpp
  Themes/RetroMediaLab/ThemeEntry.cpp
  Themes/RetroMediaLab/ThemeIcons.cpp
  Themes/RetroMediaLab/Views/MainMenu.cpp
  Themes/RetroMediaLab/Views/PreferencesDialog.cpp
  Themes/RetroMediaLab/Views/GreaseweazleDialog.cpp
  Themes/RetroMediaLab/Views/GreaseweazleToolPage.cpp
  Themes/RetroMediaLab/Views/HardwareSettingsPage.cpp
  Themes/RetroMediaLab/Views/EngineSettingsPage.cpp
  Themes/RetroMediaLab/Views/ProfileSettingsPage.cpp
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
  Themes/RetroMediaLab/Views/EmulationPage.cpp
  Themes/RetroMediaLab/Views/EmulationErrorDialog.cpp
  Themes/RetroMediaLab/Views/EmulationStoragePanel.cpp
  Themes/RetroMediaLab/Views/MachineControllerPorts.cpp
  Themes/RetroMediaLab/Views/EmulationConfigurationsPanel.cpp
  Themes/RetroMediaLab/Views/EmulationNetworkPanel.cpp
  Themes/RetroMediaLab/Views/EmulationPreferencesDialog.cpp
  Themes/RetroMediaLab/Views/EmulationControllerPanel.cpp
  Themes/RetroMediaLab/Views/ClassificationPanel.cpp
  Themes/RetroMediaLab/Views/MediaLoadingOverlay.cpp
  Themes/RetroMediaLab/Views/ExplorerPage.cpp
  Themes/RetroMediaLab/Views/VisualizationPage.cpp
  Themes/RetroMediaLab/Views/LinearGraph.cpp
  Themes/RetroMediaLab/Views/SequentialGraph.cpp
  Themes/RetroMediaLab/Views/FluxGraph.cpp
  Themes/RetroMediaLab/Views/MediaGraph.cpp
  Themes/RetroMediaLab/Views/CapturePage.cpp
  Themes/RetroMediaLab/Views/CaptureProgressView.cpp
  Themes/RetroMediaLab/Views/MigrationDialog.cpp
  Themes/RetroMediaLab/Views/ConversionPage.cpp
  Themes/RetroMediaLab/Views/ConversionDialogs.cpp
  Themes/RetroMediaLab/Views/StatusBar.cpp
  Themes/RetroMediaLab/RetroMediaLabStyle.cpp
  Themes/RetroMediaLab/RetroMediaLabLayout.cpp
  Themes/RetroMediaLab/RetroMediaLabWindow.cpp
  Themes/RetroMediaLab/RetroMediaLabPresentation.cpp)
if(MSVC)
  # Debug emits more COFF sections than the standard object format can hold.
  set_source_files_properties(Themes/RetroMediaLab/Views/EmulationPage.cpp
    PROPERTIES COMPILE_OPTIONS /bigobj)
endif()
set_target_properties(RetroMediaLab.Theme.RetroMediaLab PROPERTIES OUTPUT_NAME RetroMediaLab PREFIX "")
target_link_libraries(RetroMediaLab.Theme.RetroMediaLab PRIVATE RetroMediaLab.App RetroMediaLab.Platform RetroMediaLab.Emulation RetroMediaLab.VideoPresentation)
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
set(asset_copy_command copy_directory_if_different)
if(CMAKE_VERSION VERSION_LESS "3.26")
  set(asset_copy_command copy_directory)
endif()
add_custom_target(RetroMediaLab.Assets
  COMMAND ${CMAKE_COMMAND} -E ${asset_copy_command}
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets" "$<TARGET_FILE_DIR:RetroMediaLab>/Assets"
  VERBATIM)
add_dependencies(RetroMediaLab RetroMediaLab.Theme.RetroMediaLab RetroMediaLab.Assets)
set_target_properties(RetroMediaLab PROPERTIES OUTPUT_NAME retromedialab)
install(TARGETS RetroMediaLab RUNTIME DESTINATION . BUNDLE DESTINATION .)
install(DIRECTORY Assets DESTINATION .)
