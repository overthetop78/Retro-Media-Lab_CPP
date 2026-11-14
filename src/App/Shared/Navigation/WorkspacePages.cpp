#include "WorkspacePages.h"
namespace RetroMediaLab {
const std::array<WorkspacePage, 7>& workspacePages() {
    static const std::array<WorkspacePage, 7> pages{{
        {"read", "Tab.Read"}, {"write", "Tab.Write"}, {"convert", "Tab.Convert"},
        {"visualize", "Tab.Visualize"}, {"explorer", "Tab.Explorer"},
        {"tools", "Tab.Tools"}, {"emulation", "Tab.Emulation"}
    }};
    return pages;
}
}
