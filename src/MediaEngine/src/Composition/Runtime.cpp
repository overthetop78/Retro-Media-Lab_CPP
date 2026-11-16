#include "Runtime.h"
namespace RetroMediaLab::MediaEngine::RuntimeDetails {
std::shared_ptr<Services> defaultServices(){static const auto value=std::make_shared<Services>();return value;}
}
