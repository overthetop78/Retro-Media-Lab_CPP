#pragma once
#include <Constants/AppConstants.h>
#include "AppApi.h"
#include <array>
namespace RetroMediaLab {
struct WorkspacePage { const char* id; const char* translationKey; };
RETROMEDIALAB_APP_API const std::array<WorkspacePage, AppConstants::WorkspacePagesValues::PageCount>& workspacePages();
}
