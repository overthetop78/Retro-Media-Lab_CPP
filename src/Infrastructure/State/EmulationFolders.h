#pragma once
#include <QString>

namespace RetroMediaLab {
struct EmulationFolders {
    QString storage;
    QString captures;
    QString states;
    bool operator==(const EmulationFolders&) const = default;
};
}
