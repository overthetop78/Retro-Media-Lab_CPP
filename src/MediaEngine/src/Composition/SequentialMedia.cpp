#include "SequentialMedia.h"
#include "../Decoding/Sequential/Atari/Decoder.h"
#include "../Decoding/Sequential/Amstrad/Decoder.h"
#include "../Decoding/Sequential/Spectrum/Decoder.h"
#include "../Decoding/Sequential/Commodore/Decoder.h"
#include "../Decoding/Sequential/Msx/Decoder.h"
#include "../Decoding/Sequential/Acorn/AcornDecoder.h"
#include "../Decoding/Sequential/Acorn/AtomDecoder.h"
#include "../Encoding/Sequential/Atari/Encoder.h"
#include "../Encoding/Sequential/Amstrad/Encoder.h"
#include "../Encoding/Sequential/Spectrum/Encoder.h"
#include "../Encoding/Sequential/Commodore/Encoder.h"
#include "../Encoding/Sequential/Msx/Encoder.h"
#include "../Encoding/Sequential/Acorn/AcornEncoder.h"
#include "../Encoding/Sequential/Acorn/AtomEncoder.h"

namespace RetroMediaLab::MediaEngine {
SequentialMediaComposition SequentialMediaComposition::createDefault() {
    return {
        SequentialDecoderRegistry({
            std::make_shared<AtariTapeDecoder>(),
            std::make_shared<AmstradCpcTapeDecoder>(),
            std::make_shared<SpectrumTapeDecoder>(),
            std::make_shared<CommodoreTapeDecoder>(),
            std::make_shared<MsxTapeDecoder>(),
            std::make_shared<AcornTapeDecoder>(),
            std::make_shared<AtomTapeDecoder>()
        }),
        SequentialEncoderRegistry({
            std::make_shared<AtariTapeEncoder>(),
            std::make_shared<AmstradCpcTapeEncoder>(),
            std::make_shared<SpectrumTapeEncoder>(),
            std::make_shared<CommodoreTapeEncoder>(),
            std::make_shared<MsxTapeEncoder>(),
            std::make_shared<AcornTapeEncoder>(),
            std::make_shared<AtomTapeEncoder>()
        })
    };
}
}
