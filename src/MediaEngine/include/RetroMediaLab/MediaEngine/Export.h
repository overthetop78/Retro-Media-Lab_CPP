#pragma once
#if defined(_MSC_VER)
# define RETROMEDIALAB_MEDIAENGINE_NOVTABLE __declspec(novtable)
#else
# define RETROMEDIALAB_MEDIAENGINE_NOVTABLE
#endif
#if defined(_WIN32)
# if defined(RETROMEDIALAB_MEDIAENGINE_BUILD)
#  define RETROMEDIALAB_MEDIAENGINE_API __declspec(dllexport)
# else
#  define RETROMEDIALAB_MEDIAENGINE_API __declspec(dllimport)
# endif
#else
# define RETROMEDIALAB_MEDIAENGINE_API __attribute__((visibility("default")))
#endif
