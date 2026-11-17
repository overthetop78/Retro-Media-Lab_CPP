#pragma once
#include <Constants/AppConstants.h>
#include <QString>
#include <algorithm>
#include <functional>
#include <memory>
#include <vector>
namespace RetroMediaLab {
struct ConsoleState {
    using Subscription=std::shared_ptr<const std::function<void()>>;
    Subscription subscribe(std::function<void()> callback) {auto value=std::make_shared<const std::function<void()>>(std::move(callback));observers.push_back(value);return value;}
    void setContent(const QString& command,const QString& text) {
        if(commandPreview==command && output==text)return;
        commandPreview=command;output=text;
        const auto copy=observers;std::erase_if(observers,[](const auto& observer){return observer.expired();});
        for(const auto& observer:copy)if(const auto callback=observer.lock())(*callback)();
    }
    bool expanded = false;
    int height = AppConstants::ConsoleStateValues::DefaultConsoleHeight;
    QString commandPreview, output;
    int selectionAnchor = 0, selectionPosition = 0, scroll = 0;
    int horizontalScroll = 0;
    int commandAnchor = 0, commandPosition = 0, commandScroll = 0;
private:
    std::vector<std::weak_ptr<const std::function<void()>>> observers;
};
}
