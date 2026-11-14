#pragma once
#include "AppApi.h"
#include <array>
namespace RetroMediaLab {
struct WorkspacePage { const char* id; const char* translationKey; };
RETROMEDIALAB_APP_API const std::array<WorkspacePage, 7>& workspacePages();
}
