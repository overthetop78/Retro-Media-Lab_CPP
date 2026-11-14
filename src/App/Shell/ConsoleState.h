#pragma once
#include <Constants/AppConstants.h>
#include <QString>
namespace RetroMediaLab {
struct ConsoleState {
    bool expanded = false;
    int height = AppConstants::ConsoleStateValues::DefaultConsoleHeight;
    QString commandPreview, output;
    int selectionAnchor = 0, selectionPosition = 0, scroll = 0;
    int horizontalScroll = 0;
    int commandAnchor = 0, commandPosition = 0, commandScroll = 0;
};
}
