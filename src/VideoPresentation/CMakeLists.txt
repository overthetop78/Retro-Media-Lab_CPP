add_library(RetroMediaLab.VideoPresentation SHARED src/AnalogSignal.cpp src/CrtConnection.cpp src/CrtProcessing.cpp src/DotMatrixProcessing.cpp src/FixedPixelProcessing.cpp src/FullscreenOsd.cpp src/LedMatrixProcessing.cpp src/OpenGlSurface.cpp src/PaperProjectionProcessing.cpp src/PlasmaProcessing.cpp src/Presentation.cpp src/RestorationProcessing.cpp src/SegmentDisplayProcessing.cpp src/SoftwareProcessing.cpp src/StylisticProcessing.cpp src/VectorProcessing.cpp src/VfdProcessing.cpp)
find_package(Qt6 REQUIRED COMPONENTS GuiPrivate ShaderTools)
set(video_shader_options)
if(WIN32 AND CMAKE_HOST_WIN32)
    find_program(RETROMEDIALAB_FXC fxc NO_CACHE)
    if(RETROMEDIALAB_FXC)
        list(APPEND video_shader_options PRECOMPILE)
    endif()
endif()
qt_add_shaders(RetroMediaLab.VideoPresentation "video_sampling"
    PREFIX "/video-presentation"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
    NOGLSL NOMSL
    ${video_shader_options}
    FILES shaders/sampling.vert shaders/sampling.frag shaders/crt.frag
)
qt_add_resources(RetroMediaLab.VideoPresentation "video_adjustments"
    PREFIX "/video-presentation"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
    FILES shaders/image-adjustments.glsl shaders/image-restoration.glsl shaders/crt.glsl
)
target_sources(RetroMediaLab.VideoPresentation PRIVATE src/VulkanSurface.cpp)
target_compile_definitions(RetroMediaLab.VideoPresentation PRIVATE RETROMEDIALAB_VIDEO_PRESENTATION_BUILD)
target_include_directories(RetroMediaLab.VideoPresentation PUBLIC include)
set(vulkan_headers "${PROJECT_SOURCE_DIR}/third_party/vulkan-headers")
target_include_directories(RetroMediaLab.VideoPresentation SYSTEM PRIVATE "${vulkan_headers}/include")
install(FILES "${vulkan_headers}/LICENSE.md" DESTINATION lib/Vulkan-Headers)
install(DIRECTORY "${vulkan_headers}/LICENSES" DESTINATION lib/Vulkan-Headers)
target_link_libraries(RetroMediaLab.VideoPresentation PUBLIC Qt6::Widgets RetroMediaLab.Emulation PRIVATE Qt6::OpenGLWidgets Qt6::GuiPrivate)
set_target_properties(RetroMediaLab.VideoPresentation PROPERTIES OUTPUT_NAME VideoPresentation)
install(TARGETS RetroMediaLab.VideoPresentation RUNTIME DESTINATION lib LIBRARY DESTINATION lib)
install(FILES THIRD-PARTY-NOTICES.md DESTINATION lib RENAME VideoPresentation.NOTICES.md)

if(WIN32)
    target_sources(RetroMediaLab.VideoPresentation PRIVATE src/Direct3DSurface.cpp src/NativeRefreshClock.cpp)
    target_link_libraries(RetroMediaLab.VideoPresentation PRIVATE d3d11 d3dcompiler gdi32)
endif()
