#include <RetroMediaLab/Emulation/SessionDirectories.h>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUuid>
#include <algorithm>

namespace RetroMediaLab::Emulation {
namespace {
constexpr auto ParentDirectory = "..";
constexpr auto CurrentDirectory = ".";
constexpr auto Entries = QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot;

bool isGeneratedName(const QString& name) {
    constexpr qsizetype CompactLength = 32;
    if (name.size() == CompactLength) {
        return std::ranges::all_of(name, [](QChar digit) {
            const auto lower = digit.toLower();
            return (lower >= u'0' && lower <= u'9') || (lower >= u'a' && lower <= u'f');
        });
    }
    const auto uuid = QUuid::fromString(name);
    return name.compare(uuid.toString(QUuid::WithoutBraces), Qt::CaseInsensitive) == 0 ||
        name.compare(uuid.toString(QUuid::WithBraces), Qt::CaseInsensitive) == 0;
}

bool removeTree(const QString& directory, bool clearReadOnly) {
    bool success = true;
    const QDir root(directory);
    for (const auto& entry : root.entryInfoList(Entries, QDir::NoSort)) {
        const auto path = entry.absoluteFilePath();
        bool removed;
        if (entry.isJunction()) {
            removed = QDir().rmdir(path);
        } else if (entry.isDir() && !entry.isSymLink()) {
            removed = removeTree(path, clearReadOnly);
        } else {
#ifdef Q_OS_WIN
            if (clearReadOnly && entry.isFile() && !entry.isSymLink() && !(entry.permissions() & QFileDevice::WriteOwner))
                QFile::setPermissions(path, entry.permissions() | QFileDevice::WriteOwner);
#endif
            removed = QFile::remove(path);
        }
        success = success && removed;
    }
    return QDir().rmdir(directory) && success;
}

bool removeOwnedDirectory(const QString& sessionRoot, const QString& directory, bool generated) {
    if (sessionRoot.trimmed().isEmpty() || directory.trimmed().isEmpty()) return false;
    const QFileInfo rootInfo(sessionRoot);
    const QDir root(rootInfo.absoluteFilePath());
    const auto canonicalRoot = root.canonicalPath();
    if (canonicalRoot.isEmpty()) return false;
    const auto target = QDir::cleanPath(root.absoluteFilePath(QDir::fromNativeSeparators(directory)));
    const auto relative = root.relativeFilePath(target);
    const auto parts = relative.split(u'/');
    if (QDir::isAbsolutePath(relative) || parts.front() == QLatin1String(ParentDirectory) ||
            parts.front() == QLatin1String(CurrentDirectory) || relative.isEmpty() ||
            (generated && !isGeneratedName(parts.back()))) return false;

    auto owned = canonicalRoot;
    for (const auto& part : parts) {
        owned = QDir(owned).filePath(part);
        const QFileInfo info(owned);
        if (info.isSymLink() || info.isJunction()) return false;
        if (!info.exists()) return true;
        if (!info.isDir()) return false;
    }
    return removeTree(owned, generated);
}
}

bool removeSessionDirectory(const QString& sessionRoot, const QString& directory) {
    return removeOwnedDirectory(sessionRoot, directory, false);
}

bool removeGeneratedSessionDirectory(const QString& sessionRoot, const QString& directory) {
    return removeOwnedDirectory(sessionRoot, directory, true);
}

}
