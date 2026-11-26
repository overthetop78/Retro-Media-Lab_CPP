add_library(RetroMediaLab.Emulation SHARED src/Host.cpp src/AudioOutput.cpp src/MediaPreparation.cpp src/StateFile.cpp
  src/ControllerFeedback.cpp src/Firmware.cpp src/HostProtocol.cpp src/HostedCoreTransport.cpp src/HostedCoreSession.cpp
  src/StorageSettings.cpp src/InputFrames.cpp src/InputProcessing.cpp src/Networks.cpp src/ExecutionRate.cpp
  src/SessionDirectories.cpp include/RetroMediaLab/Emulation/SessionDirectories.h
  src/HardwareSettings.cpp include/RetroMediaLab/Emulation/HardwareSettings.h
  src/ConfigurationFiles.cpp include/RetroMediaLab/Emulation/ConfigurationFiles.h
  src/CoreReleases.cpp include/RetroMediaLab/Emulation/CoreReleases.h
  src/NativeCoreHardwareRender.cpp include/RetroMediaLab/Emulation/NativeCoreHardwareRender.h
  src/Messages.cpp include/RetroMediaLab/Emulation/Messages.h
  src/MachineExecution.cpp src/CoreDiagnostics.cpp src/NativeCoreContent.cpp src/NativeCoreLibrary.cpp src/NativeCoreFrames.cpp src/NativeCoreEnvironment.cpp src/NativeCoreOptions.cpp
  include/RetroMediaLab/Emulation/CoreDiagnostics.h
  include/RetroMediaLab/Emulation/NativeCoreContent.h
  include/RetroMediaLab/Emulation/NativeCoreLibrary.h
  include/RetroMediaLab/Emulation/NativeCoreFrames.h
  src/AudioBuffer.cpp include/RetroMediaLab/Emulation/AudioBuffer.h
  src/AudioOutputController.cpp include/RetroMediaLab/Emulation/AudioOutputController.h
  src/AudioProcessing.cpp include/RetroMediaLab/Emulation/AudioProcessing.h
  include/RetroMediaLab/Emulation/NativeCoreEnvironment.h
  include/RetroMediaLab/Emulation/NativeCoreOptions.h
  src/NativeCoreFileSystem.cpp include/RetroMediaLab/Emulation/NativeCoreFileSystem.h
  src/NativeCoreDiskControl.cpp include/RetroMediaLab/Emulation/NativeCoreDiskControl.h
  src/NativeCoreSession.cpp include/RetroMediaLab/Emulation/NativeCoreSession.h
  include/RetroMediaLab/Emulation/ExecutionRate.h
  include/RetroMediaLab/Emulation/MachineExecution.h
  include/RetroMediaLab/Emulation/Networks.h
  include/RetroMediaLab/Emulation/EmulationKey.h)
set_target_properties(RetroMediaLab.Emulation PROPERTIES AUTOMOC ON)
target_compile_definitions(RetroMediaLab.Emulation PRIVATE RETROMEDIALAB_EMULATION_BUILD)
target_include_directories(RetroMediaLab.Emulation PUBLIC include)
target_link_libraries(RetroMediaLab.Emulation PUBLIC Qt6::Core PRIVATE RetroMediaLab.MediaEngine RetroMediaLab.Archives Qt6::Multimedia Qt6::Network)
if(WIN32)
  target_link_libraries(RetroMediaLab.Emulation PRIVATE opengl32 gdi32 user32)
endif()
set_target_properties(RetroMediaLab.Emulation PROPERTIES OUTPUT_NAME Emulation)
install(TARGETS RetroMediaLab.Emulation RUNTIME DESTINATION lib LIBRARY DESTINATION lib)
