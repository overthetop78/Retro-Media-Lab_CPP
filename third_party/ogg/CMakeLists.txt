add_library(RetroMediaLab.Ogg STATIC src/bitwise.c src/framing.c)
target_include_directories(RetroMediaLab.Ogg PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
