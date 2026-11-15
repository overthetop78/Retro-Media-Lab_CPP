#include "RecognitionResult.h"
namespace RetroMediaLab::MediaEngine {
const char* RecognitionError::what()const noexcept{return diagnosticKey(diagnostic_.code).data();}
}
