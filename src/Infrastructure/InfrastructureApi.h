#pragma once
#if defined(_WIN32)
# if defined(RETROMEDIALAB_INFRASTRUCTURE_BUILD)
#  define RETROMEDIALAB_INFRASTRUCTURE_API __declspec(dllexport)
# else
#  define RETROMEDIALAB_INFRASTRUCTURE_API __declspec(dllimport)
# endif
#else
# define RETROMEDIALAB_INFRASTRUCTURE_API __attribute__((visibility("default")))
#endif
