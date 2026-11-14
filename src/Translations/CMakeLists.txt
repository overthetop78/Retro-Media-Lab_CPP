add_library(RetroMediaLab.Translations SHARED
  src/TranslationService.cpp
)
target_compile_definitions(RetroMediaLab.Translations PRIVATE RETROMEDIALAB_TRANSLATIONS_BUILD)
set_target_properties(RetroMediaLab.Translations PROPERTIES OUTPUT_NAME Translations)
target_include_directories(RetroMediaLab.Translations PUBLIC include)
target_link_libraries(RetroMediaLab.Translations PRIVATE Qt6::Core)
install(TARGETS RetroMediaLab.Translations RUNTIME DESTINATION lib LIBRARY DESTINATION lib)

find_package(Python3 3.10 REQUIRED COMPONENTS Interpreter)
function(add_translation_catalog name directory)
  file(GLOB_RECURSE catalog_sources CONFIGURE_DEPENDS "${directory}/*.json")
  set(generated "${CMAKE_CURRENT_BINARY_DIR}/generated/${name}.cpp")
  add_custom_command(OUTPUT "${generated}"
    COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/scripts/translations/compile_catalog.py"
      --source "${directory}" --output "${generated}"
    DEPENDS ${catalog_sources} "${PROJECT_SOURCE_DIR}/scripts/translations/compile_catalog.py"
    VERBATIM)
  add_library("RetroMediaLab.Catalog.${name}" SHARED "${generated}")
  if(name STREQUAL "Constants")
    set(output_name "constants")
  else()
    set(output_name "${name}")
  endif()
  set_target_properties("RetroMediaLab.Catalog.${name}" PROPERTIES OUTPUT_NAME "${output_name}" PREFIX "")
  if(MSVC)
    set_target_properties("RetroMediaLab.Catalog.${name}" PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  endif()
  install(TARGETS "RetroMediaLab.Catalog.${name}" RUNTIME DESTINATION lib/Languages LIBRARY DESTINATION lib/Languages)
endfunction()
add_translation_catalog(Constants "${CMAKE_CURRENT_SOURCE_DIR}/Catalogs/Constants")
file(GLOB languages LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/Catalogs/Languages/*")
foreach(language IN LISTS languages)
  if(IS_DIRECTORY "${language}")
    get_filename_component(name "${language}" NAME)
    add_translation_catalog("${name}" "${language}")
  endif()
endforeach()
