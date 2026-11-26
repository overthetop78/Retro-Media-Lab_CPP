#include <Constants/AppConstants.h>
#include "WorkspacePages.h"
namespace RetroMediaLab {
const std::array<WorkspacePage, AppConstants::WorkspacePagesValues::PageCount>& workspacePages() {
    static const std::array<WorkspacePage, AppConstants::WorkspacePagesValues::PageCount> pages{{
        {AppConstants::Read, AppConstants::TabRead}, {AppConstants::Write, AppConstants::TabWrite}, {AppConstants::Convert, AppConstants::TabConvert},
        {AppConstants::Visualize, AppConstants::TabVisualize}, {AppConstants::Explorer, AppConstants::TabExplorer},
        {AppConstants::Emulation, AppConstants::TabEmulation}
    }};
    return pages;
}
}
