#pragma once

#include "EmulationExport.h"
#include "Sdk.h"
#include <exception>
#include <stdexcept>

namespace RetroMediaLab::Emulation {
enum class MessageCategory { Emulator, Machine, Firmware, Media, Input, Video, Audio, SavedState };
enum class MessageCode {
    UntranslatedEmulatorMessage, EmulatorNotInstalled, EmulatorRejected, EmulatorUpdateBlocked,
    RequiredMediaMissing, FirmwareMissing, FirmwareIncompatible, MediaNotFound, MediaUnsupported,
    OptionInvalid, HostCommunicationFailed, MachineStartFailed, MediaOperationFailed,
    SavedStateInvalid, SavedStateIncompatible, SavedStateOperationFailed
};
enum class MessageSeverity { Information, Warning, Error };
enum class MessageTarget { Silent, Status, Dialog };

struct MessageContext {
    QString emulatorId;
    QString machineId;
    QList<MediaKind> requiredMedia;
    QStringList requiredMediaNames;
};

struct EmulationMessage {
    MessageCategory category;
    MessageCode code;
    MessageSeverity severity = MessageSeverity::Error;
    MessageTarget target = MessageTarget::Dialog;
    MessageContext context;
    QString originalText;
};

class RETROMEDIALAB_EMULATION_API EmulationMessageException final : public std::runtime_error {
public:
    explicit EmulationMessageException(EmulationMessage message, std::exception_ptr cause = {});
    const EmulationMessage& message() const noexcept;
    const std::exception_ptr& cause() const noexcept;

private:
    EmulationMessage message_;
    std::exception_ptr cause_;
};

RETROMEDIALAB_EMULATION_API std::exception_ptr translateError(std::exception_ptr error,
    MessageCategory category, MessageCode code, MessageContext context = {});
RETROMEDIALAB_EMULATION_API std::exception_ptr translateLocalizedError(std::exception_ptr error,
    MessageCategory category, MessageContext context = {});
}
