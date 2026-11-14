#pragma once
#if defined(_WIN32)
# if defined(RETROMEDIALAB_TRANSLATIONS_BUILD)
#  define RETROMEDIALAB_TRANSLATIONS_API __declspec(dllexport)
# else
#  define RETROMEDIALAB_TRANSLATIONS_API __declspec(dllimport)
# endif
#else
# define RETROMEDIALAB_TRANSLATIONS_API __attribute__((visibility("default")))
#endif
