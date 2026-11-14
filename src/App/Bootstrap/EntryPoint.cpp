#include "Application.h"
#include <QDebug>
#include <exception>
#include <cstdio>
#if defined(_WIN32)
# define RETROMEDIALAB_ENTRY __declspec(dllexport)
#else
# define RETROMEDIALAB_ENTRY
#endif
extern "C" RETROMEDIALAB_ENTRY int RetroMediaLabRun(int argc, char** argv) {
    try { return RetroMediaLab::Application{}.run(argc, argv); }
    catch (const std::exception& error) {
        std::fprintf(stderr, "Retro Media Lab: %s\n", error.what());
        qCritical("Retro Media Lab: %s", error.what());
    }
    catch (...) {
        std::fprintf(stderr, "Retro Media Lab: unexpected application exception\n");
        qCritical("Retro Media Lab: unexpected application exception");
    }
    return 1;
}
