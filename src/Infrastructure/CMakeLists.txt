add_library(RetroMediaLab.Infrastructure SHARED
  Media/ExplorerSession.cpp
  Media/OpeningProgress.cpp
  Media/MigrationSession.cpp
  Media/ClassificationSelection.cpp
  Media/ExplorerReadSession.cpp
  Media/VisualizationSession.cpp
  Media/SimulatedDriveAudio.cpp
  Media/MediaCommands.cpp
  Media/CaptureSession.cpp
  Media/GwProgressTracker.cpp
  Media/ConversionSession.cpp
  Media/ConversionConflicts.cpp
  Profiles/OperationProfiles.cpp
  State/PreferencesStore.cpp
  Engines/EngineSettings.cpp
  Maintenance/GreaseweazleTools.cpp
  Hardware/HardwareSettings.cpp
  Hardware/HardwareAvailability.cpp
  Hardware/GreaseweazleDiscovery.cpp
  Hardware/HostToolsService.cpp
  Hardware/OpticalProbe.cpp
  Hardware/DiskProbe.cpp
  Hardware/DiskProbeService.cpp
  Conversion/ConversionSettings.cpp
  Logging/LogSettings.cpp
  Logging/ConsoleLogSession.cpp
  Logging/DiagnosticLog.cpp
  Logging/LogPaths.cpp)
target_compile_definitions(RetroMediaLab.Infrastructure PRIVATE RETROMEDIALAB_INFRASTRUCTURE_BUILD RETROMEDIALAB_APP_VERSION="${PROJECT_VERSION}")
target_include_directories(RetroMediaLab.Infrastructure PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" include)
target_link_libraries(RetroMediaLab.Infrastructure PUBLIC Qt6::Core RetroMediaLab.Runtime RetroMediaLab.MediaEngine
  PRIVATE RetroMediaLab.Translations RetroMediaLab.Platform Qt6::Network Qt6::Multimedia RetroMediaLab.Archives)
if(WIN32)
  target_sources(RetroMediaLab.Infrastructure PRIVATE Windows/HardwareDiscovery.cpp Windows/StorageCommands.cpp Windows/ElevatedDiskProbe.cpp)
  target_link_libraries(RetroMediaLab.Infrastructure PRIVATE shell32 ole32 uuid setupapi advapi32 propsys cfgmgr32)
else()
  target_sources(RetroMediaLab.Infrastructure PRIVATE Unix/HardwareDiscovery.cpp)
endif()
set_target_properties(RetroMediaLab.Infrastructure PROPERTIES OUTPUT_NAME Infrastructure PREFIX "")
install(TARGETS RetroMediaLab.Infrastructure RUNTIME DESTINATION lib LIBRARY DESTINATION lib)
