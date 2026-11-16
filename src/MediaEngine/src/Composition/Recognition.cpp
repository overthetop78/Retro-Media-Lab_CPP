#include "Recognition.h"
#include "../Formats/Adf/Adapter.h"
#include "../Formats/Cartridge/Readers.h"
#include "../Formats/AcornAtom/Reader.h"
#include "../Formats/Apridisk/Adapter.h"
#include "../Formats/Definitions/RawImages.h"
#include "../Formats/BbcDfs/Adapter.h"
#include "../Formats/Raw/SpecializedReaders.h"
#include "../Formats/Rx02/Adapter.h"
#include "../Formats/Atari/Adapters.h"
#include "../Formats/Xfd/Reader.h"
#include "../Formats/CommodoreDisks/Adapters.h"
#include "../Formats/Apple/Adapter.h"
#include "../Formats/Raw/Adapters.h"
#include "../Formats/CpcDsk/Adapter.h"
#include "../Formats/Raw/ImgReader.h"
#include "../Formats/TeleDisk/Reader.h"
#include "../Formats/I86f/I86f.h"
#include "../Formats/Cp2/Cp2.h"
#include "../Formats/ImageDisk/Adapter.h"
#include "../Formats/Jfd/Adapter.h"
#include "../Formats/Epson/RawReader.h"
#include "../Formats/Hfe/Adapter.h"
#include "../Formats/HxcStream/Adapter.h"
#include "../Formats/KryoFlux/Adapter.h"
#include "../Formats/Scp/Adapter.h"
#include "../Formats/Optical/BinCue/Reader.h"
#include "../Formats/Optical/CloneCd/Reader.h"
#include "../Formats/Optical/Alcohol/Reader.h"
#include "../Formats/Optical/Chd/Reader.h"
#include "../Formats/Optical/Gdi/Container.h"
#include "../Formats/Optical/Xdvdfs/Reader.h"
#include "../Formats/Optical/WiiU/Reader.h"
#include "../Formats/Optical/Iso/Reader.h"
#include "../Formats/Tape/Wav/Reader.h"
#include "../Formats/Tape/Csw/Reader.h"
#include "../Formats/Tape/Raw/Reader.h"
#include "../Formats/Tape/Audio/Reader.h"
#include "../Formats/Tape/Voc/Reader.h"
#include "../Formats/Tape/Uef/Reader.h"
#include "../Formats/Tape/AtariCas/Reader.h"
#include "../Formats/Tape/Tzx/Reader.h"
#include "../Formats/Tape/CommodoreTap/Reader.h"
#include "../Formats/Tape/MsxCas/Reader.h"
#include "../Formats/Tape/SpectrumTap/Reader.h"
#include "../Formats/Tape/AtomTap/Reader.h"
#include "../Formats/Tape/Simh/Reader.h"
#include "../Formats/HardDisk/Qcow2/Container.h"
#include "../Formats/HardDisk/Vhdx/Container.h"
#include "../Formats/HardDisk/Vhd/Container.h"
#include "../Formats/HardDisk/Vdi/Container.h"
#include "../Formats/HardDisk/Vmdk/Container.h"
#include "../Formats/HardDisk/Chd/Container.h"
#include "../Formats/HardDisk/Acorn/Container.h"
#include "../Formats/HardDisk/MfmEmu/Adapter.h"
#include "../Formats/HardDisk/Raw/Reader.h"
#include <QString>
#include <set>
namespace RetroMediaLab::MediaEngine {
MediaRecognitionComposition::MediaRecognitionComposition(std::shared_ptr<const Scp::Reader> scp,std::vector<std::shared_ptr<const ImageReader>> readers)
:scp_(std::move(scp)),registry_(std::move(readers)){
    std::set<std::string> unique;
    for(const auto& reader:registry_.readers())for(const auto& extension:reader->capabilities().extensions)unique.insert(QString::fromStdString(extension).toCaseFolded().toStdString());
    extensions_.assign(unique.begin(),unique.end());
}
MediaRecognitionComposition MediaRecognitionComposition::createDefault(){
    auto scp=std::make_shared<Scp::Reader>();
    return {scp,{
        std::make_shared<Adf::Reader>(),
        std::make_shared<Cartridge::CprReader>(),
        std::make_shared<Cartridge::RomReader>(),
        std::make_shared<Cartridge::ConsoleReader>(),
        std::make_shared<Cartridge::FdsReader>(),
        std::make_shared<AcornAtom::Reader>(),
        std::make_shared<Apridisk::Reader>(),
        Definitions::rawReader(),
        std::make_shared<BbcDfs::Reader>(),
        std::make_shared<Raw::SpecializedReader>(Raw::SpecializedKind::Coherent),
        std::make_shared<Rx02::Reader>(),
        std::make_shared<Atari::Reader>(Atari::Kind::St),
        std::make_shared<Atari::Reader>(Atari::Kind::Msa),
        std::make_shared<Atari::Reader>(Atari::Kind::Atr),
        std::make_shared<Xfd::Reader>(),
        std::make_shared<Atari::Reader>(Atari::Kind::Atx),
        std::make_shared<CommodoreDisks::ImageReaderAdapter>(CommodoreDisks::Kind::D64),
        std::make_shared<CommodoreDisks::ImageReaderAdapter>(CommodoreDisks::Kind::D71),
        std::make_shared<CommodoreDisks::ImageReaderAdapter>(CommodoreDisks::Kind::D81),
        std::make_shared<Apple::Reader>(),
        std::make_shared<Raw::Reader>(Raw::ReaderKind::Msx),
        std::make_shared<CpcDsk::Reader>(),
        std::make_shared<Raw::ImgReader>(),
        std::make_shared<Raw::Reader>(Raw::ReaderKind::Ibm),
        std::make_shared<TeleDisk::Reader>(),
        std::make_shared<I86f::Reader>(),
        std::make_shared<Cp2::Reader>(),
        std::make_shared<ImageDisk::Reader>(),
        std::make_shared<Jfd::Reader>(),
        std::make_shared<Epson::RawReader>(),
        std::make_shared<Raw::SpecializedReader>(Raw::SpecializedKind::Ucsd),
        std::make_shared<Hfe::Reader>(),
        std::make_shared<HxcStream::Reader>(),
        std::make_shared<KryoFlux::Reader>(),
        scp,
        std::make_shared<BinCue::Reader>(),
        std::make_shared<CloneCd::Reader>(),
        std::make_shared<AlcoholMds::Reader>(),
        std::make_shared<ChdOptical::Reader>(),
        std::make_shared<Gdi::Reader>(),
        std::make_shared<Xdvdfs::Reader>(),
        std::make_shared<WiiU::Reader>(),
        std::make_shared<OpticalIso::Reader>(),
        std::make_shared<WavTape::Reader>(),
        std::make_shared<Csw::Reader>(),
        std::make_shared<RawTape::Reader>(),
        std::make_shared<AudioTape::Reader>(),
        std::make_shared<VocTape::Reader>(),
        std::make_shared<Uef::Reader>(),
        std::make_shared<AtariCas::Reader>(),
        std::make_shared<Tzx::Reader>(),
        std::make_shared<CommodoreTap::Reader>(),
        std::make_shared<MsxCas::Reader>(),
        std::make_shared<SpectrumTap::Reader>(),
        std::make_shared<AtomTap::Reader>(),
        std::make_shared<SimhTape::Reader>(),
        std::make_shared<Qcow2::Reader>(),
        std::make_shared<Vhdx::Reader>(),
        std::make_shared<Vhd::Reader>(),
        std::make_shared<Vdi::Reader>(),
        std::make_shared<Vmdk::Reader>(),
        std::make_shared<ChdHardDisk::Reader>(),
        std::make_shared<AcornWinchester::Reader>(),
        std::make_shared<MfmEmu::Reader>(),
        std::make_shared<HardDiskRaw::Reader>(),
    }};
}
}
