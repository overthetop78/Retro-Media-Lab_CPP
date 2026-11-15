add_library(RetroMediaLab.MediaAnalysis SHARED
  # Common
  src/Classifier.cpp
  src/Recognition.cpp)
target_compile_definitions(RetroMediaLab.MediaAnalysis PRIVATE RETROMEDIALAB_MEDIAANALYSIS_BUILD)
target_include_directories(RetroMediaLab.MediaAnalysis PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(RetroMediaLab.MediaAnalysis PRIVATE Qt6::Core)
set_target_properties(RetroMediaLab.MediaAnalysis PROPERTIES OUTPUT_NAME MediaAnalysis PREFIX "")
install(TARGETS RetroMediaLab.MediaAnalysis RUNTIME DESTINATION lib LIBRARY DESTINATION lib)
