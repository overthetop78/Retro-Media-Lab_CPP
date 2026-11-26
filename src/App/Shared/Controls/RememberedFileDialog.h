#pragma once
#include "RememberedFileDialogConstants.h"
#include "State/WorkspaceState.h"
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>

namespace RetroMediaLab {
class RememberedFileDialog {
public:
    static void prepare(QFileDialog& dialog, const WorkspaceState& state, const QString& scope,
                        const QString& fallback = {}) {
        auto directory = state.fileDialogDirectories().value(scope).toString();
        if ((directory.isEmpty() || !QDir(directory).exists()) && !fallback.isEmpty())
            directory = QFileInfo(fallback).isDir() ? fallback : QFileInfo(fallback).absolutePath();
        if (!directory.isEmpty() && QDir(directory).exists()) dialog.setDirectory(directory);
    }

    static void remember(QFileDialog& dialog, WorkspaceState& state, const QString& scope) {
        QObject::connect(&dialog, &QFileDialog::directoryEntered, &dialog, [&state, scope](const QString& directory) {
            save(state, scope, directory);
        });
        QObject::connect(&dialog, &QFileDialog::finished, &dialog, [&dialog, &state, scope](int result) {
            const auto selection = dialog.selectedFiles();
            if (result == QDialog::Accepted && !selection.isEmpty()) {
                const auto& path = selection.front();
                save(state, scope, dialog.fileMode() == QFileDialog::Directory ? path : QFileInfo(path).absolutePath());
            } else {
                save(state, scope, dialog.directory().absolutePath());
            }
        });
    }

    static QString choose(QWidget* parent, WorkspaceState& state, const QString& title, const QString& scope,
                          const QString& fallback, QFileDialog::FileMode mode = QFileDialog::ExistingFile,
                          bool saving = false) {
        QFileDialog dialog(parent, title);
        dialog.setFileMode(mode);
        dialog.setAcceptMode(saving ? QFileDialog::AcceptSave : QFileDialog::AcceptOpen);
        if (mode == QFileDialog::Directory) dialog.setOption(QFileDialog::ShowDirsOnly);
        prepare(dialog, state, scope, fallback);
        remember(dialog, state, scope);
        if (dialog.exec() != QDialog::Accepted) return {};
        return dialog.selectedFiles().value(0);
    }

private:
    static void save(WorkspaceState& state, const QString& scope, const QString& directory) {
        if (!directory.isEmpty() && QDir(directory).exists()) state.setFileDialogDirectory(scope, directory);
    }
};
}
