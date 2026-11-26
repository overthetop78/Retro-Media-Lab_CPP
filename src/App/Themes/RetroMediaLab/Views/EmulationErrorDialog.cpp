#include "EmulationErrorDialog.h"
#include "MediaPageConstants.h"
#include "Shared/Controls/Card.h"
#include "Shared/Presentation/AssetPath.h"

#include <RetroMediaLab/Emulation/Messages.h>
#include <RetroMediaLab/Translations/TranslationService.h>
#include <QDebug>
#include <QDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QStyle>
#include <QVBoxLayout>

namespace RetroMediaLab {
namespace {
using namespace Emulation;
using Translation = Translations::TranslationService;
constexpr auto UnknownKey = "Common.Unknown";
constexpr auto ErrorTitleKey = "Emulation.Error.Title";
constexpr auto PowerFailureKey = "Emulation.Error.PowerFailureTitle";
constexpr auto MachineKey = "Emulation.Error.Machine";
constexpr auto RequiredMediaKey = "Emulation.Error.RequiredMedia";
constexpr auto ConfirmCaption = "OK";
constexpr auto MediaSeparator = " / ";
constexpr int MinimumWidth = 480;
constexpr int MaximumWidth = 680;
constexpr int BodyMargin = 26;
constexpr int TopMargin = 24;
constexpr int FooterMargin = 18;
constexpr int DescriptionSpacing = 10;
constexpr int DetailSpacing = 16;
constexpr int ButtonWidth = 110;
constexpr int MediaColumnWidth = 104;
constexpr int MediaPanelHeight = 176;
constexpr int MaxMediaIcon = 76;
constexpr int MessageIconSize = 34;
constexpr int HeadingPointSize = 15;
constexpr int ScreenInset = 24;

struct MediaPresentation {
    const char* nameKey;
    const char* icon;
};

MediaPresentation describeMedia(MediaKind kind) {
    using namespace MediaPageConstants;
    switch (kind) {
    case MediaKind::Floppy: return {"Emulation.Storage.Floppy.Device", FloppyIcon};
    case MediaKind::HardDisk: return {"Emulation.Storage.HardDisk.Device", "data"};
    case MediaKind::OpticalDisc: return {"Emulation.Storage.CompactDiscs", OpticalIcon};
    case MediaKind::Tape: return {"Emulation.Storage.Tapes", TapeIcon};
    case MediaKind::Cartridge: return {"Emulation.Storage.Cartridges", "rom-memory"};
    case MediaKind::MemoryCard: return {"Emulation.Storage.Media.Associated", "rom-memory"};
    }
    return {UnknownKey, FileIcon};
}

QString text(const Translation& translations, const char* key) {
    return QString::fromStdString(translations.text(key));
}

const char* errorResource(MessageCode code) {
    switch (code) {
    case MessageCode::EmulatorNotInstalled: return "Emulation.Error.EmulatorNotInstalled";
    case MessageCode::EmulatorRejected: return "Emulation.Error.EmulatorRejected";
    case MessageCode::EmulatorUpdateBlocked: return "Emulation.Error.EmulatorUpdateBlocked";
    case MessageCode::RequiredMediaMissing: return "Emulation.Error.RequiredMediaMissing";
    case MessageCode::FirmwareMissing: return "Emulation.Error.FirmwareMissing";
    case MessageCode::FirmwareIncompatible: return "Emulation.Error.FirmwareIncompatible";
    case MessageCode::MediaNotFound: return "Emulation.Error.MediaNotFound";
    case MessageCode::MediaUnsupported: return "Emulation.Error.MediaUnsupported";
    case MessageCode::OptionInvalid: return "Emulation.Error.OptionInvalid";
    case MessageCode::HostCommunicationFailed: return "Emulation.Error.HostCommunicationFailed";
    case MessageCode::MachineStartFailed: return "Emulation.Error.MachineStartFailed";
    case MessageCode::MediaOperationFailed: return "Emulation.Error.MediaOperationFailed";
    case MessageCode::SavedStateInvalid: return "Emulation.Error.SavedStateInvalid";
    case MessageCode::SavedStateIncompatible: return "Emulation.Error.SavedStateIncompatible";
    case MessageCode::SavedStateOperationFailed: return "Emulation.Error.SavedStateOperationFailed";
    case MessageCode::UntranslatedEmulatorMessage: return "Common.Unknown";
    }
    return UnknownKey;
}

QLabel* plainLabel(const QString& contents, QWidget* parent) {
    auto* label = new QLabel(contents, parent);
    label->setTextFormat(Qt::PlainText);
    label->setWordWrap(true);
    return label;
}

QStyle::StandardPixmap severityIcon(MessageSeverity severity) {
    switch (severity) {
    case MessageSeverity::Information: return QStyle::SP_MessageBoxInformation;
    case MessageSeverity::Warning: return QStyle::SP_MessageBoxWarning;
    case MessageSeverity::Error: return QStyle::SP_MessageBoxCritical;
    }
    return QStyle::SP_MessageBoxCritical;
}
}

QString presentEmulationError(QWidget& owner, const std::shared_ptr<const ThemeData>& theme,
    const Translation& translations, const EmulationMessageException& error, const QString& machineName) {
    const auto& message = error.message();
    const auto description = message.originalText.trimmed().isEmpty()
        ? text(translations, errorResource(message.code)) : message.originalText;
    auto mediaNames = message.context.requiredMediaNames;
    if (mediaNames.isEmpty()) {
        for (const auto kind : message.context.requiredMedia)
            mediaNames.append(text(translations, describeMedia(kind).nameKey));
    }
    const auto media = mediaNames.join(QLatin1String(MediaSeparator));
    {
        const auto level = message.severity == MessageSeverity::Information ? QtInfoMsg :
            message.severity == MessageSeverity::Warning ? QtWarningMsg : QtCriticalMsg;
        auto log = QDebug(level);
        log.noquote() << errorResource(message.code) << machineName << description << media;
        if (const auto& cause = error.cause()) {
            try { std::rethrow_exception(cause); }
            catch (const std::exception& native) { log << QString::fromUtf8(native.what()); }
            catch (...) {}
        }
    }
    if (message.target == MessageTarget::Silent) return {};
    if (message.target == MessageTarget::Status) return description;

    const bool powerFailure = message.code == MessageCode::RequiredMediaMissing ||
        message.code == MessageCode::MachineStartFailed;
    const auto title = text(translations, powerFailure ? PowerFailureKey : ErrorTitleKey);
    auto* window = owner.window();
    QDialog dialog(window, Qt::Dialog | Qt::FramelessWindowHint);
    dialog.setWindowTitle(title);
    dialog.setAttribute(Qt::WA_TranslucentBackground);
    auto* root = new QVBoxLayout(&dialog);
    root->setContentsMargins(0, 0, 0, 0);
    auto* card = new Card(theme, &dialog);
    root->addWidget(card);

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(BodyMargin, TopMargin, BodyMargin, FooterMargin);
    auto* body = new QHBoxLayout;
    body->setSpacing(DetailSpacing);
    layout->addLayout(body);

    QStringList icons;
    for (const auto kind : message.context.requiredMedia) {
        const auto icon = QString::fromLatin1(describeMedia(kind).icon);
        if (!icons.contains(icon)) icons.append(icon);
    }
    if (icons.isEmpty()) {
        auto* icon = new QLabel(card);
        icon->setPixmap(dialog.style()->standardIcon(severityIcon(message.severity))
            .pixmap(MessageIconSize, MessageIconSize));
        body->addWidget(icon, 0, Qt::AlignTop);
    } else {
        auto* column = new QWidget(card);
        column->setFixedWidth(MediaColumnWidth);
        auto* iconLayout = new QVBoxLayout(column);
        iconLayout->setContentsMargins(0, 0, 0, 0);
        const auto size = qMin(MaxMediaIcon, MediaPanelHeight / static_cast<int>(icons.size()));
        for (const auto& name : icons) {
            auto* icon = new QLabel(column);
            const auto path = QString::fromLatin1(MediaPageConstants::ResourceIconPath).arg(name);
            icon->setPixmap(QIcon(assetPath(path)).pixmap(size, size));
            iconLayout->addWidget(icon, 0, Qt::AlignHCenter);
        }
        body->addWidget(column, 0, Qt::AlignVCenter);
    }

    auto* copy = new QVBoxLayout;
    copy->setSpacing(DescriptionSpacing);
    body->addLayout(copy, 1);
    auto* heading = plainLabel(title, card);
    auto font = heading->font();
    font.setPointSize(qMax(font.pointSize(), HeadingPointSize));
    font.setWeight(QFont::DemiBold);
    heading->setFont(font);
    copy->addWidget(heading);
    auto* label = plainLabel(description, card);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    copy->addWidget(label);
    if (!media.isEmpty()) {
        copy->addSpacing(DetailSpacing);
        auto* details = new QFormLayout;
        details->setRowWrapPolicy(QFormLayout::WrapLongRows);
        details->addRow(plainLabel(text(translations, MachineKey), card), plainLabel(machineName, card));
        details->addRow(plainLabel(text(translations, RequiredMediaKey), card), plainLabel(media, card));
        copy->addLayout(details);
    }
    copy->addStretch();

    auto* confirm = new QPushButton(QLatin1String(ConfirmCaption), card);
    confirm->setDefault(true);
    confirm->setMinimumWidth(ButtonWidth);
    layout->addWidget(confirm, 0, Qt::AlignRight);
    QObject::connect(confirm, &QPushButton::clicked, &dialog, &QDialog::accept);

    const auto bounds = window->screen()->availableGeometry();
    const auto width = qMin(MaximumWidth, qMax(1, bounds.width() - ScreenInset));
    dialog.setMinimumWidth(qMin(MinimumWidth, width));
    dialog.setMaximumWidth(width);
    dialog.adjustSize();
    auto position = window->frameGeometry().center() - dialog.rect().center();
    position.setX(qBound(bounds.left(), position.x(), qMax(bounds.left(), bounds.right() - dialog.width() + 1)));
    position.setY(qBound(bounds.top(), position.y(), qMax(bounds.top(), bounds.bottom() - dialog.height() + 1)));
    dialog.move(position);
    confirm->setFocus();
    dialog.exec();
    return {};
}
}
