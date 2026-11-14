#pragma once
#if defined(_WIN32)
# if defined(RETROMEDIALAB_APP_BUILD)
#  define RETROMEDIALAB_APP_API __declspec(dllexport)
# else
#  define RETROMEDIALAB_APP_API __declspec(dllimport)
# endif
#else
# define RETROMEDIALAB_APP_API __attribute__((visibility("default")))
#endif
