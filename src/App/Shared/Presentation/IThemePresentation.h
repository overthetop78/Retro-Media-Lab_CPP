#pragma once
namespace RetroMediaLab {
// A complete presentation may own one window, a desktop or several independent windows.
// No tabs, navigation layout or native frame are imposed by this contract.
class IThemePresentation {
public:
    virtual ~IThemePresentation() = default;
    virtual void synchronizeState() = 0;
    virtual void show() = 0;
};
}
