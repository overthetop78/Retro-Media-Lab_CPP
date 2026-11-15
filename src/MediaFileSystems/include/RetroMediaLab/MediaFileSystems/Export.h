#pragma once
#if defined(_WIN32)
# if defined(RETROMEDIALAB_MEDIAFILESYSTEMS_BUILD)
#  define RETROMEDIALAB_MEDIAFILESYSTEMS_API __declspec(dllexport)
# else
#  define RETROMEDIALAB_MEDIAFILESYSTEMS_API __declspec(dllimport)
# endif
#else
# define RETROMEDIALAB_MEDIAFILESYSTEMS_API __attribute__((visibility("default")))
#endif
