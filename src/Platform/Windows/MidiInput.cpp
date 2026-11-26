#include <RetroMediaLab/Platform/MidiInput.h>
#include <QUrl>
#include <QtCore/qt_windows.h>
#include <mmsystem.h>
#include <array>
#include <atomic>
#include <bit>
#include <mutex>
#include <vector>

namespace RetroMediaLab::Platform {
namespace {
constexpr quint32 CommandMask = 0xf0, ChannelMask = 0x0f, DataMask = 0x7f;
constexpr quint32 NoteOff = 0x80, NoteOn = 0x90, ControlChange = 0xb0;
constexpr quint32 AllSoundOff = 120, AllNotesOff = 123;
constexpr int Channels = 16, NotesPerChannel = 128, WordBits = 64, WordsPerChannel = NotesPerChannel / WordBits;
constexpr int DataShift = 8, ValueShift = 16;
constexpr auto DeviceIdFormat = "midi:%1:%2";
constexpr auto ControlFormat = "Ch%1-N%2";

QString deviceId(const QString& name, int occurrence) {
    return QString::fromLatin1(DeviceIdFormat).arg(QString::fromLatin1(QUrl::toPercentEncoding(name)),
        QString::number(occurrence));
}

class HeldNotes final {
public:
    void receive(quint32 message) noexcept {
        const auto command = message & CommandMask;
        const auto channel = message & ChannelMask;
        const auto note = (message >> DataShift) & DataMask;
        const auto velocity = (message >> ValueShift) & DataMask;
        const auto bit = quint64{1} << (note % WordBits);
        auto& word = notes_[channel][note / WordBits];
        if (command == NoteOn && velocity != 0) word.fetch_or(bit, std::memory_order_relaxed);
        else if (command == NoteOff || command == NoteOn) word.fetch_and(~bit, std::memory_order_relaxed);
        else if (command == ControlChange && (note == AllSoundOff || note == AllNotesOff))
            for (auto& channelWord : notes_[channel]) channelWord.store(0, std::memory_order_relaxed);
    }

    QHash<QString, float> controls() const {
        QHash<QString, float> controls;
        for (int channel = 0; channel < Channels; ++channel) {
            for (int word = 0; word < WordsPerChannel; ++word) {
                auto held = notes_[channel][word].load(std::memory_order_relaxed);
                while (held != 0) {
                    const int note = word * WordBits + std::countr_zero(held);
                    controls.insert(QString::fromLatin1(ControlFormat).arg(channel + 1).arg(note), 1.f);
                    held &= held - 1;
                }
            }
        }
        return controls;
    }

private:
    static_assert(std::atomic<quint64>::is_always_lock_free);
    std::array<std::array<std::atomic<quint64>, WordsPerChannel>, Channels> notes_{};
};

class Port final {
public:
    Port(QString id, UINT index) : id_(std::move(id)), index_(index) {
        if (midiInOpen(&input_, index, reinterpret_cast<DWORD_PTR>(&callback),
                reinterpret_cast<DWORD_PTR>(&notes_), CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
            input_ = nullptr;
            return;
        }
        if (midiInStart(input_) != MMSYSERR_NOERROR) close();
    }
    ~Port() { close(); }
    Port(const Port&) = delete;
    Port& operator=(const Port&) = delete;

    bool opened() const { return input_ != nullptr; }
    GamepadState snapshot() const { return {index_, 0, {}, id_, notes_.controls()}; }

private:
    static void CALLBACK callback(HMIDIIN, UINT kind, DWORD_PTR context, DWORD_PTR message, DWORD_PTR) noexcept {
        if (kind == MIM_DATA) reinterpret_cast<HeldNotes*>(context)->receive(static_cast<quint32>(message));
    }

    void close() noexcept {
        if (!input_) return;
        midiInStop(input_);
        midiInClose(input_);
        input_ = nullptr;
    }

    const QString id_;
    const UINT index_;
    HeldNotes notes_;
    HMIDIIN input_ = nullptr;
};
}

struct MidiInput::State {
    State() {
        QHash<QString, int> occurrences;
        const UINT count = midiInGetNumDevs();
        for (UINT index = 0; index < count; ++index) {
            MIDIINCAPSW capabilities{};
            if (midiInGetDevCapsW(index, &capabilities, sizeof(capabilities)) != MMSYSERR_NOERROR) continue;
            const auto name = QString::fromWCharArray(capabilities.szPname);
            const int occurrence = ++occurrences[name.toCaseFolded()];
            auto port = std::make_unique<Port>(deviceId(name, occurrence), index);
            if (port->opened()) ports.push_back(std::move(port));
        }
    }

    std::vector<std::unique_ptr<Port>> ports;
};

MidiInput::MidiInput() {
    static std::mutex gate;
    static std::weak_ptr<State> shared;
    const std::lock_guard lock(gate);
    state_ = shared.lock();
    if (!state_) shared = state_ = std::make_shared<State>();
}

QList<GamepadState> MidiInput::read() const {
    QList<GamepadState> samples;
    samples.reserve(qsizetype(state_->ports.size()));
    for (const auto& port : state_->ports) samples.append(port->snapshot());
    return samples;
}
}
