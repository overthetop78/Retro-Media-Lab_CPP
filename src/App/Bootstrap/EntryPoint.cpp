#include <ProcessConstants.h>
#include <ErrorKeys.h>
#include "Application.h"
#include "Logging/DiagnosticLog.h"
#include "Logging/LogPaths.h"
#include "Logging/LogConstants.h"
#include <QFileInfo>
#include <ErrorDialog.h>
#include <exception>
#include <cstdio>
#if defined(_WIN32)
# define RETROMEDIALAB_ENTRY __declspec(dllexport)
#else
# define RETROMEDIALAB_ENTRY
#endif
namespace {
void logFailure(const char* process,const char* message) noexcept {
    try {
        const auto path=QString::fromUtf8(process);
        RetroMediaLab::writeDiagnostic(RetroMediaLab::logsDirectory(QFileInfo(path).absolutePath()),RetroMediaLab::DiagnosticSeverity::Error,
            QString::fromUtf8(message),RetroMediaLab::LogConstants::ApplicationContext,
            QString::fromStdString(RetroMediaLab::Errors::presentationLanguage),path);
    }catch(...){} // A failed diagnostic must not hide the original exception.
}
}
extern "C" RETROMEDIALAB_ENTRY int RetroMediaLabRun(int argc, char** argv) {
    try { return RetroMediaLab::Application{}.run(argc, argv); }
    catch (const std::exception& error) {
        logFailure(argv[0],error.what());
        RetroMediaLab::Errors::show(RetroMediaLab::Errors::presentationLanguage, error.what());
    }
    catch (...) {
        const auto message=RetroMediaLab::Errors::render(RetroMediaLab::Errors::presentationLanguage,RetroMediaLab::ErrorKeys::UnexpectedApplicationException);
        logFailure(argv[0],message.c_str());
        RetroMediaLab::Errors::show(RetroMediaLab::Errors::presentationLanguage,message);
    }
    return RetroMediaLab::ProcessConstants::Failure;
}
