#pragma once
#if defined(_WIN32)
# if defined(RETROMEDIALAB_MEDIAANALYSIS_BUILD)
#  define RETROMEDIALAB_MEDIAANALYSIS_API __declspec(dllexport)
# else
#  define RETROMEDIALAB_MEDIAANALYSIS_API __declspec(dllimport)
# endif
#else
# define RETROMEDIALAB_MEDIAANALYSIS_API __attribute__((visibility("default")))
#endif
