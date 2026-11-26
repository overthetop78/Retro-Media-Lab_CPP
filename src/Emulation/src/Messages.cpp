#include <RetroMediaLab/Emulation/Messages.h>

#include <system_error>
#include <utility>

namespace RetroMediaLab::Emulation {
namespace {
constexpr auto UnspecifiedMessage = "Emulation.Error.Title";

std::exception_ptr wrap(std::exception_ptr error, EmulationMessage message, bool preserveText) {
    if (!error) return {};
    try {
        std::rethrow_exception(error);
    } catch (const EmulationMessageException&) {
        return error;
    } catch (const std::system_error& failure) {
        if (failure.code() == std::errc::operation_canceled) return error;
        if (preserveText) message.originalText = QString::fromUtf8(failure.what());
    } catch (const std::exception& failure) {
        if (preserveText) message.originalText = QString::fromUtf8(failure.what());
    } catch (...) {
    }
    return std::make_exception_ptr(EmulationMessageException(std::move(message), error));
}
}

EmulationMessageException::EmulationMessageException(EmulationMessage message, std::exception_ptr cause)
    : std::runtime_error(message.originalText.isEmpty() ? UnspecifiedMessage : message.originalText.toStdString()),
      message_(std::move(message)), cause_(std::move(cause)) {}

const EmulationMessage& EmulationMessageException::message() const noexcept { return message_; }
const std::exception_ptr& EmulationMessageException::cause() const noexcept { return cause_; }

std::exception_ptr translateError(std::exception_ptr error, MessageCategory category, MessageCode code,
    MessageContext context) {
    return wrap(error, {category, code, MessageSeverity::Error, MessageTarget::Dialog, std::move(context), {}}, false);
}

std::exception_ptr translateLocalizedError(std::exception_ptr error, MessageCategory category, MessageContext context) {
    return wrap(error, {category, MessageCode::UntranslatedEmulatorMessage, MessageSeverity::Error,
        MessageTarget::Dialog, std::move(context), {}}, true);
}
}
