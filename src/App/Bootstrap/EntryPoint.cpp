#include <ProcessConstants.h>
#include <ErrorKeys.h>
#include "Application.h"
#include <ErrorDialog.h>
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
        RetroMediaLab::Errors::show(RetroMediaLab::Errors::presentationLanguage, error.what());
    }
    catch (...) {
        RetroMediaLab::Errors::show(RetroMediaLab::Errors::presentationLanguage,
            RetroMediaLab::Errors::render(RetroMediaLab::Errors::presentationLanguage, RetroMediaLab::ErrorKeys::UnexpectedApplicationException));
    }
    return RetroMediaLab::ProcessConstants::Failure;
}
