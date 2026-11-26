#include "EmulationPreferencesDialog.h"
#include "EmulationNetworkPanel.h"
#include "Shared/Emulation/EmulationWorkspace.h"
#include <ErrorKeys.h>
#include "Shared/Emulation/EmulationKeyboardShortcuts.h"
#include "Shared/Controls/EmulationShortcutCapture.h"
#include "State/WorkspaceState.h"
#include <RetroMediaLab/Translations/TranslationService.h>

#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeySequenceEdit>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSet>
#include <QSignalBlocker>
#include <QTabWidget>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>
#include <utility>

namespace RetroMediaLab {
namespace {
namespace Keys = EmulationKeyboardShortcuts;
constexpr auto PreferencesKey = "Menu.EmulationPreferences";
constexpr auto ShortcutsKey = "Emulation.Tab.Shortcuts";
constexpr auto GeneralKey = "Emulation.Tab.General";
constexpr auto BrowseKey = "Common.Browse";
constexpr auto CloseKey = "Common.Close";
constexpr int FolderMargin = 12, FolderBrowseWidth = 100;

struct FolderField {
    const char* key;
    QString EmulationFolders::* member;
};

constexpr FolderField FolderFields[]{
    {"Emulation.Folder.StorageBase", &EmulationFolders::storage},
    {"Emulation.Folder.Capture", &EmulationFolders::captures},
    {"Emulation.Folder.State", &EmulationFolders::states},
};
constexpr auto ActionsKey = "Emulation.Input.Actions";
constexpr auto BindingKey = "Emulation.Input.Binding.Label";
constexpr auto SearchKey = "Emulation.Input.Binding.Search";
constexpr auto AssignKey = "Emulation.Input.Assign";
constexpr auto DeleteKey = "Common.Delete";
constexpr auto CaptureHintKey = "Emulation.Input.Capture.Hint";
constexpr auto PressKey = "Emulation.Input.Press";
constexpr auto ConflictKey = "Emulation.Input.Binding.LabelConflict";
constexpr auto ReservedKey = "Emulation.Input.Binding.LabelReserved";
constexpr auto RestoreKey = "Emulation.Input.Binding.RestoreDefaults";
constexpr auto ClearConflictsKey = "Emulation.Input.Binding.ClearConflicts";
constexpr auto AlternativeLabelSuffix = "Alternative";
constexpr int ActionColumn = 0;
constexpr int BindingColumn = 1;
constexpr int FirstActionRow = 1;

struct ShortcutRow {
    const Keys::Binding& binding;
    const char* label;
};

constexpr ShortcutRow ShortcutRows[] = {
    {Keys::OpenSettings, "Emulation.Machine.Settings"},
    {Keys::ReleaseMouse, "Emulation.Shortcut.ReleaseMouse"},
    {Keys::PauseResume, "Emulation.Shortcut.PauseResume"},
    {Keys::ToggleFullscreen, "Emulation.Shortcut.Fullscreen"},
    {Keys::Power, "Emulation.Shortcut.Power"},
    {Keys::SoftReset, "Emulation.Shortcut.SoftReset"},
    {Keys::HardReset, "Emulation.Shortcut.HardReset"},
    {Keys::QuickSave, "Emulation.Shortcut.QuickSave"},
    {Keys::QuickLoad, "Emulation.Shortcut.QuickLoad"},
    {Keys::Screenshot, "Emulation.Shortcut.Screenshot"},
    {Keys::ToggleMute, "Emulation.Shortcut.Mute"},
    {Keys::OsdWithPause, "Emulation.Shortcut.OsdWithPause"},
    {Keys::OsdWithoutPause, "Emulation.Shortcut.OsdWithoutPause"},
    {Keys::OsdStatistics, "Emulation.Shortcut.OsdStatistics"},
    {Keys::OsdUp, "Emulation.Shortcut.OsdUp"},
    {Keys::OsdDown, "Emulation.Shortcut.OsdDown"},
    {Keys::OsdLeft, "Emulation.Shortcut.OsdLeft"},
    {Keys::OsdRight, "Emulation.Shortcut.OsdRight"},
    {Keys::OsdConfirm, "Emulation.Shortcut.OsdConfirm"},
    {Keys::OsdCancel, "Emulation.Shortcut.OsdCancel"},
};

struct EditorRow {
    QLabel* label;
    QKeySequenceEdit* editor;
    QWidget* controls;
    QPushButton* assign;
    QToolButton* clear;
    QString id;
    QKeySequence fallback;
};
}

void EmulationPreferencesDialog::open(QWidget* parent, Translations::TranslationService& translations, WorkspaceState& state,
        const VideoPresentation::ProfileStore& profiles, EditSavedConfiguration edit) {
    QList<std::pair<FolderField, QLineEdit*>> folderInputs;
    QDialog dialog(parent);
    const auto text = [&](const char* key) { return QString::fromStdString(translations.text(key)); };
    dialog.setWindowTitle(text(PreferencesKey));
    auto* layout = new QVBoxLayout(&dialog);
    auto* tabs = new QTabWidget(&dialog);
    layout->addWidget(tabs, 1);
    auto* general = new QScrollArea(tabs);
    general->setWidgetResizable(true);
    auto* folderContent = new QWidget(general);
    auto* folders = new QFormLayout(folderContent);
    folders->setContentsMargins(FolderMargin, FolderMargin, FolderMargin, FolderMargin);
    auto* folderError = new QLabel(folderContent);
    folderError->setWordWrap(true);
    folderError->hide();
    const auto saveFolder = [&state, folderError, &translations, &folderInputs](const FolderField& field, QLineEdit& input) {
        try {
            auto values = state.emulationFolders();
            values.*field.member = input.text();
            state.setEmulationFolders(std::move(values));
            const auto resolved = resolveEmulationFolders(state.emulationFolders());
            for (const auto& [entry, editor] : std::as_const(folderInputs)) {
                const QSignalBlocker blocker(editor);
                editor->setText(resolved.*entry.member);
                editor->setModified(false);
            }
            folderError->hide();
        } catch (...) {
            input.setModified(true);
            folderError->setText(QString::fromStdString(translations.text(ErrorKeys::CannotSavePreferences)));
            folderError->show();
        }
    };
    for (const auto& field : FolderFields) {
        auto* row = new QWidget(folderContent);
        auto* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins({});
        auto* input = new QLineEdit(resolveEmulationFolders(state.emulationFolders()).*field.member, row);
        auto* browse = new QPushButton(text(BrowseKey), row);
        browse->setMinimumWidth(FolderBrowseWidth);
        auto* label = new QLabel(text(field.key), folderContent);
        label->setBuddy(input);
        rowLayout->addWidget(input, 1);
        rowLayout->addWidget(browse);
        folders->addRow(label, row);
        folderInputs.append({field, input});
        QObject::connect(input, &QLineEdit::editingFinished, row, [input, field, saveFolder] {
            if (input->isModified()) saveFolder(field, *input);
        });
        QObject::connect(browse, &QPushButton::clicked, row, [&dialog, label, input, field, saveFolder] {
            const auto selected = QFileDialog::getExistingDirectory(&dialog, label->text(), input->text());
            if (selected.isEmpty()) return;
            input->setText(selected);
            saveFolder(field, *input);
        });
    }
    folders->addRow(folderError);
    general->setWidget(folderContent);
    tabs->addTab(general, text(GeneralKey));
    auto* page = new QWidget(tabs);
    tabs->addTab(page, text(ShortcutsKey));
    tabs->addTab(createEmulationConfigurationsPanel(tabs, translations, state, profiles, std::move(edit)),
        text("Emulation.Configurations"));
    auto* network = createEmulationNetworkPanel(tabs, translations);
    tabs->addTab(network, text(EmulationNetworkTitleKey));
    const auto networkTitle = translations.subscribe([&] {
        tabs->setTabText(tabs->indexOf(network), text(EmulationNetworkTitleKey));
    });
    auto* pageLayout = new QVBoxLayout(page);
    auto* search = new QLineEdit(page);
    search->setPlaceholderText(text(SearchKey));
    search->setClearButtonEnabled(true);
    auto* toolbar = new QHBoxLayout;
    auto* restore = new QPushButton(text(RestoreKey), page);
    auto* clearConflicts = new QPushButton(text(ClearConflictsKey), page);
    toolbar->addWidget(search, 1);
    toolbar->addWidget(restore);
    toolbar->addWidget(clearConflicts);
    pageLayout->addLayout(toolbar);
    auto* scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    pageLayout->addWidget(scroll, 1);
    auto* content = new QWidget(scroll);
    scroll->setWidget(content);
    auto* grid = new QGridLayout(content);
    grid->setColumnStretch(ActionColumn, 1);
    grid->addWidget(new QLabel(text(ActionsKey), content), 0, ActionColumn);
    grid->addWidget(new QLabel(text(BindingKey), content), 0, BindingColumn);
    auto* conflictMessage = new QLabel(page);
    conflictMessage->setWordWrap(true);
    conflictMessage->hide();
    pageLayout->addWidget(conflictMessage);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    buttons->button(QDialogButtonBox::Close)->setText(text(CloseKey));
    layout->addWidget(buttons);
    auto draft = state.emulationShortcuts();
    QList<EditorRow> rows;
    for (const auto& row : ShortcutRows) {
        const auto add = [&](const QString& labelKey, const QString& id, const QKeySequence& fallback) {
            const auto stored = draft.value(id);
            auto* controls = new QWidget(content);
            auto* rowLayout = new QHBoxLayout(controls);
            rowLayout->setContentsMargins(0, 0, 0, 0);
            auto* editor = new QKeySequenceEdit(stored.isUndefined() ? fallback : Keys::parse(stored.toString()), controls);
            rowLayout->addWidget(editor, 1);
            auto* assign = new QPushButton(text(AssignKey), controls);
            rowLayout->addWidget(assign);
            auto* clear = new QToolButton(controls);
            clear->setIcon(clear->style()->standardIcon(QStyle::SP_DialogDiscardButton));
            clear->setToolTip(text(DeleteKey));
            clear->setAccessibleName(text(DeleteKey));
            rowLayout->addWidget(clear);
            editor->setFinishingKeyCombinations({});
            editor->setMaximumSequenceLength(1);
            auto* label = new QLabel(QString::fromStdString(translations.text(labelKey.toStdString())), content);
            label->setBuddy(editor);
            const auto rowIndex = int(rows.size()) + FirstActionRow;
            grid->addWidget(label, rowIndex, ActionColumn);
            grid->addWidget(controls, rowIndex, BindingColumn);
            rows.append({label, editor, controls, assign, clear, id, fallback});
        };
        add(QLatin1String(row.label), QLatin1String(row.binding.id), QKeySequence(row.binding.defaultKey));
        if (row.binding.hasAlternative)
            add(QLatin1String(row.label) + QLatin1String(AlternativeLabelSuffix),
                QLatin1String(row.binding.id) + QLatin1String(Keys::AlternativeSuffix), {});
    }
    grid->setRowStretch(int(rows.size()) + FirstActionRow, 1);
    QSet<QString> invalidBindings;
    bool saveFailed = false;
    const auto refresh = [&] {
        const auto query = search->text().trimmed();
        QList<std::pair<QKeySequence, QString>> used;
        QList<std::pair<EmulationPhysicalShortcuts::Chord, QString>> usedPhysical;
        QStringList conflicts;
        QStringList reserved;
        invalidBindings.clear();
        const auto addConflict = [&](const QString& previousId, const QString& previous,
                const EditorRow& row, const QString& binding) {
            invalidBindings.insert(previousId);
            invalidBindings.insert(row.id);
            conflicts.append(previous);
            conflicts.append(binding);
        };
        for (const auto& row : std::as_const(rows)) {
            const auto binding = draft.value(row.id);
            const auto stored = binding.toString();
            const auto sequence = binding.isUndefined() ? row.fallback : Keys::parse(stored);
            auto physical = EmulationPhysicalShortcuts::chord(stored, true);
            if (physical.reserved || (!stored.trimmed().isEmpty() && sequence.isEmpty() && physical.isEmpty())) {
                reserved.append(stored);
                invalidBindings.insert(row.id);
            }
            if (!physical.isEmpty()) {
                for (const auto& [previous, id] : std::as_const(usedPhysical)) {
                    if (!EmulationPhysicalShortcuts::conflicts(previous, physical)) continue;
                    addConflict(id, draft.value(id).toString(), row, stored);
                }
                usedPhysical.append(std::pair{std::move(physical), row.id});
            }
            if (auto* input = row.editor->findChild<QLineEdit*>())
                input->setPlaceholderText(sequence.isEmpty() && !stored.isEmpty() ? stored : text(PressKey));
            row.clear->setEnabled(!sequence.isEmpty() || !stored.isEmpty());
            if (!sequence.isEmpty()) {
                if (Keys::reserved(sequence)) {
                    reserved.append(sequence.toString());
                    invalidBindings.insert(row.id);
                }
                for (const auto& [previous, id] : std::as_const(used)) {
                    if (!Keys::conflicts(previous, sequence)) continue;
                    addConflict(id, previous.toString(), row, sequence.toString());
                }
                used.append({sequence, row.id});
            }
            const bool visible = row.label->text().contains(query, Qt::CaseInsensitive)
                || sequence.toString().contains(query, Qt::CaseInsensitive) || stored.contains(query, Qt::CaseInsensitive);
            row.label->setVisible(visible);
            row.controls->setVisible(visible);
        }
        QStringList issues;
        const auto addIssues = [&](const char* key, QStringList& sequences) {
            if (sequences.isEmpty()) return;
            sequences.removeDuplicates();
            issues.append(text(key) + QLatin1String(": ") + sequences.join(QLatin1String(", ")));
        };
        addIssues(ConflictKey, conflicts);
        addIssues(ReservedKey, reserved);
        clearConflicts->setEnabled(!invalidBindings.isEmpty());
        if (saveFailed) issues.append(text(ErrorKeys::CannotSavePreferences));
        conflictMessage->setVisible(!issues.isEmpty());
        conflictMessage->setText(issues.join(QLatin1Char('\n')));
    };
    const auto persist = [&] {
        saveFailed = false;
        refresh();
        if (!invalidBindings.isEmpty()) return;
        try {
            state.setEmulationShortcuts(draft);
        } catch (...) {
            saveFailed = true;
            refresh();
        }
    };
    for (const auto& row : std::as_const(rows)) {
        const auto setBinding = [&, row](const QString& value) {
            draft.insert(row.id, value);
            const QSignalBlocker blocker(row.editor);
            row.editor->setKeySequence(Keys::parse(value));
            persist();
        };
        QObject::connect(row.clear, &QToolButton::clicked, &dialog, [setBinding] { setBinding({}); });
        QObject::connect(row.assign, &QPushButton::clicked, &dialog, [&, setBinding, label = row.label, id = row.id] {
            EmulationShortcutCapture capture(&dialog, label->text(), text(CaptureHintKey), true, false,
                EmulationPhysicalShortcuts::isOsdShortcut(id));
            if (capture.exec() == QDialog::Accepted) setBinding(capture.binding());
        });
        QObject::connect(row.editor, &QKeySequenceEdit::editingFinished, &dialog, [&, row] {
            const auto value = row.editor->keySequence();
            const auto stored = draft.value(row.id);
            const auto previous = stored.isUndefined() ? row.fallback : Keys::parse(stored.toString());
            if (value == previous) return;
            draft.insert(row.id, value.toString(QKeySequence::PortableText));
            persist();
        });
    }
    QObject::connect(search, &QLineEdit::textChanged, &dialog, refresh);
    QObject::connect(restore, &QPushButton::clicked, &dialog, [&] {
        for (const auto& row : std::as_const(rows)) {
            draft.remove(row.id);
            const QSignalBlocker blocker(row.editor);
            row.editor->setKeySequence(row.fallback);
        }
        persist();
    });
    QObject::connect(clearConflicts, &QPushButton::clicked, &dialog, [&] {
        for (const auto& row : std::as_const(rows)) {
            if (!invalidBindings.contains(row.id)) continue;
            draft.insert(row.id, QString{});
            const QSignalBlocker blocker(row.editor);
            row.editor->clear();
        }
        persist();
    });
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    refresh();
    dialog.exec();
}
}
