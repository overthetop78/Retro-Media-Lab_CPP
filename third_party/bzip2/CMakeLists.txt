add_library(RetroMediaLab.Bzip2 STATIC blocksort.c huffman.c crctable.c randtable.c compress.c decompress.c bzlib.c)
target_include_directories(RetroMediaLab.Bzip2 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(RetroMediaLab.Bzip2 PROPERTIES POSITION_INDEPENDENT_CODE ON)
