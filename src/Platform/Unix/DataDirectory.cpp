#include <RetroMediaLab/Platform/DataDirectory.h>
#include <QStandardPaths>
namespace RetroMediaLab::Platform {
std::filesystem::path userDataDirectory() {
    return std::filesystem::path(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation).toStdString());
}
}
