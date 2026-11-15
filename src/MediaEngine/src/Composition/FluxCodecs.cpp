#include "FluxCodecs.h"
#include "../Codecs/Iso/Mfm.h"
#include "../Codecs/Iso/Fm.h"
#include "../Codecs/Amiga/Mfm.h"
#include "../Codecs/AppleII/Gcr.h"
#include "../Codecs/AppleRwts18/Gcr.h"
#include "../Codecs/AppleIwm/Gcr.h"
#include "../Codecs/Commodore/Commodore1541.h"
#include "../Codecs/HpMmfm/Mmfm.h"
#include "../Codecs/DataGeneral/Fm.h"
#include "../Codecs/Micropolis/Mfm.h"
#include "../Codecs/Membrain/Mfm.h"
#include "../Codecs/Aed6200p/Mfm.h"
#include "../Codecs/QdMo5/Mfm.h"
#include "../Codecs/Centurion/Mfm.h"
#include "../Codecs/Northstar/Mfm.h"
#include "../Codecs/Heathkit/Fm.h"
#include "../Codecs/MicralN/Fm.h"
#include "../Codecs/Emu/Fm.h"
#include "../Codecs/Tycom/Fm.h"
#include "../Codecs/DecRx02/Rx02.h"
#include "../Codecs/Arburg/Blocks.h"
#include "../Codecs/Victor9000/Gcr.h"
#include "../Codecs/Commodore/Commodore900.h"
#include "../Codecs/Raw/Raw.h"
namespace RetroMediaLab::MediaEngine::Composition {
std::vector<std::shared_ptr<const Decoding::Decoder>> defaultDecoders() {
    return {
        std::make_shared<IsoMfm::Decoder>(),
        std::make_shared<IsoFm::Decoder>(),
        std::make_shared<AmigaMfm::Decoder>(),
        std::make_shared<AppleII::Decoder>(),
        std::make_shared<AppleRwts18::Decoder>(),
        std::make_shared<AppleIwm::Decoder>(AppleIwm::Variant::Macintosh),
        std::make_shared<AppleIwm::Decoder>(AppleIwm::Variant::LisaFileWare),
        std::make_shared<Commodore1541Gcr::Decoder>(),
        std::make_shared<HpMmfm::Decoder>(),
        std::make_shared<DataGeneral::Decoder>(),
        std::make_shared<Micropolis::Decoder>(),
        std::make_shared<Membrain::Decoder>(),
        std::make_shared<Aed6200p::Decoder>(),
        std::make_shared<QdMo5::Decoder>(),
        std::make_shared<Centurion::Decoder>(),
        std::make_shared<Northstar::Decoder>(),
        std::make_shared<Heathkit::Decoder>(),
        std::make_shared<MicralN::Decoder>(),
        std::make_shared<EmuFm::Decoder>(),
        std::make_shared<Tycom::Decoder>(),
        std::make_shared<DecRx02::Decoder>(),
        std::make_shared<Arburg::Decoder>(),
        std::make_shared<Victor9000::Decoder>(),
        std::make_shared<Commodore900Gcr::Decoder>(),
        std::make_shared<RawFlux::Decoder>()
    };
}
std::vector<std::shared_ptr<const Encoding::TrackEncoder>> defaultEncoders() {
    return {
        std::make_shared<IsoMfm::Encoder>(),
        std::make_shared<IsoFm::Encoder>(),
        std::make_shared<AmigaMfm::Encoder>(),
        std::make_shared<AppleII::Encoder>(),
        std::make_shared<AppleRwts18::Encoder>(),
        std::make_shared<AppleIwm::Encoder>(AppleIwm::Variant::Macintosh),
        std::make_shared<AppleIwm::Encoder>(AppleIwm::Variant::LisaFileWare),
        std::make_shared<Commodore1541Gcr::Encoder>(),
        std::make_shared<HpMmfm::Encoder>(),
        std::make_shared<DataGeneral::Encoder>(),
        std::make_shared<Micropolis::Encoder>(),
        std::make_shared<Membrain::Encoder>(),
        std::make_shared<Aed6200p::Encoder>(),
        std::make_shared<QdMo5::Encoder>(),
        std::make_shared<Centurion::Encoder>(),
        std::make_shared<Northstar::Encoder>(),
        std::make_shared<Heathkit::Encoder>(),
        std::make_shared<MicralN::Encoder>(),
        std::make_shared<EmuFm::Encoder>(),
        std::make_shared<Tycom::Encoder>(),
        std::make_shared<DecRx02::Encoder>(),
        std::make_shared<Arburg::Encoder>(),
        std::make_shared<Victor9000::Encoder>(),
        std::make_shared<Commodore900Gcr::Encoder>()
    };
}
}

