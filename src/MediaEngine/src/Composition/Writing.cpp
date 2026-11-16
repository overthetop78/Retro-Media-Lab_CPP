#include "Writing.h"
#include "../Formats/Jfd/Adapter.h"
#include "../Formats/HxcStream/Adapter.h"
#include "../Formats/Definitions/RawImages.h"
#include "../Formats/Cartridge/Readers.h"
#include "../Formats/AcornAtom/WriterAdapter.h"
#include "../Formats/Apridisk/Adapter.h"
#include "../Formats/Adf/WriterAdapters.h"
#include "../Formats/BbcDfs/WriterAdapter.h"
#include "../Formats/Raw/WriterAdapters.h"
#include "../Formats/Apple/WriterAdapters.h"
#include "../Formats/Atari/Adapters.h"
#include "../Formats/CommodoreDisks/Adapters.h"
#include "../Formats/Commodore900/WriterAdapter.h"
#include "../Formats/CpcDsk/WriterAdapter.h"
#include "../Formats/Epson/WriterAdapter.h"
#include "../Formats/ImageDisk/WriterAdapter.h"
#include "../Formats/Rx02/Adapter.h"
#include "../Formats/Raw/UcsdWriter.h"
#include "../Formats/Scp/Adapter.h"
#include "../Formats/Hfe/Adapter.h"
#include "../Formats/HardDisk/Raw/Writer.h"
#include "../Formats/HardDisk/Acorn/Container.h"
#include "../Formats/HardDisk/MfmEmu/Adapter.h"
#include "../Formats/HardDisk/Qcow2/Container.h"
#include "../Formats/HardDisk/Vhd/Container.h"
#include "../Formats/HardDisk/Vhdx/Container.h"
#include "../Formats/HardDisk/Vdi/Container.h"
#include "../Formats/HardDisk/Vmdk/Container.h"
#include "../Formats/HardDisk/Chd/Container.h"
#include "../Formats/Optical/Iso/Writer.h"
#include "../Formats/Optical/BinCue/Reader.h"
#include "../Formats/Optical/Gdi/Container.h"
#include "../Formats/Optical/Xdvdfs/Reader.h"
#include "../Formats/Optical/WiiU/Reader.h"
#include "../Formats/Tape/Wav/Reader.h"
#include "../Formats/Tape/Csw/Reader.h"
#include "../Formats/Tape/Raw/Reader.h"
#include "../Formats/Tape/Voc/Reader.h"
#include "../Formats/Tape/AtariCas/Reader.h"
#include "../Formats/Tape/Tzx/Reader.h"
#include "../Formats/Tape/SpectrumTap/Reader.h"
#include "../Formats/Tape/AtomTap/Reader.h"
#include "../Formats/Tape/CommodoreTap/Reader.h"
#include "../Formats/Tape/MsxCas/Reader.h"
#include "../Formats/Tape/Uef/Reader.h"
#include "../Formats/Tape/Simh/Reader.h"
namespace RetroMediaLab::MediaEngine {
MediaWritingComposition::MediaWritingComposition(std::vector<std::shared_ptr<const ImageWriter>> writers,MediaDriveService drives):registry_(std::move(writers)),writing_(registry_,std::move(drives)){}
MediaWritingComposition MediaWritingComposition::createDefault(MediaDriveService drives){
    return MediaWritingComposition({
        Jfd::writerAdapter(),
        HxcStream::writerAdapter(),
        Definitions::rawWriter(),
        std::make_shared<Cartridge::Writer>(Cartridge::Kind::Cpr),
        std::make_shared<Cartridge::Writer>(Cartridge::Kind::Rom),
        std::make_shared<Cartridge::ConsoleWriter>(),
        std::make_shared<Cartridge::Writer>(Cartridge::Kind::Fds),
        AcornAtom::writerAdapter(),
        Apridisk::writerAdapter(),
        Adf::acornWriterAdapter(),
        Adf::amigaWriterAdapter(),
        BbcDfs::writerAdapter(),
        Raw::ibmWriterAdapter(),
        Raw::msxWriterAdapter(),
        Apple::sectorWriterAdapter(),
        Apple::nibbleWriterAdapter(),
        Apple::macintoshWriterAdapter(),
        Atari::stWriterAdapter(),
        Atari::atrWriterAdapter(),
        CommodoreDisks::d81WriterAdapter(),
        CommodoreDisks::dosWriterAdapter(),
        Commodore900::writerAdapter(),
        CpcDsk::writerAdapter(),
        Epson::writerAdapter(),
        ImageDisk::writerAdapter(),
        Rx02::writerAdapter(),
        Raw::ucsdWriterAdapter(),
        Scp::writerAdapter(),
        Hfe::writerAdapter(),
        std::make_shared<HardDiskRaw::Writer>(),
        std::make_shared<AcornWinchester::Writer>(),
        std::make_shared<MfmEmu::Writer>(),
        std::make_shared<Qcow2::Writer>(),
        std::make_shared<Vhd::Writer>(),
        std::make_shared<Vhdx::Writer>(),
        std::make_shared<Vdi::Writer>(),
        std::make_shared<Vmdk::Writer>(),
        std::make_shared<ChdHardDisk::Writer>(),
        std::make_shared<OpticalIso::Writer>(),
        std::make_shared<BinCue::Writer>(),
        std::make_shared<Gdi::Writer>(),
        std::make_shared<Xdvdfs::Writer>(),
        std::make_shared<WiiU::Writer>(),
        std::make_shared<WavTape::Writer>(),
        std::make_shared<Csw::Writer>(),
        std::make_shared<RawTape::Writer>(),
        std::make_shared<VocTape::Writer>(),
        std::make_shared<AtariCas::Writer>(),
        std::make_shared<Tzx::Writer>(),
        std::make_shared<SpectrumTap::Writer>(),
        std::make_shared<AtomTap::Writer>(),
        std::make_shared<CommodoreTap::Writer>(),
        std::make_shared<MsxCas::Writer>(),
        std::make_shared<Uef::Writer>(),
        std::make_shared<SimhTape::Writer>(),
    },std::move(drives));
}
}
