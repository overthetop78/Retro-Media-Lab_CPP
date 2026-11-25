#include "CatalogConstants.h"
#include "AdfsDocument.h"
#include <RetroMediaLab/MediaFileSystems/SectorReader.h>
#include <RetroMediaLab/MediaFileSystems/AmigaFlatArchive.h>
#include <RetroMediaLab/MediaFileSystems/BbcDfs.h>
#include <RetroMediaLab/MediaFileSystems/Coherent.h>
#include <RetroMediaLab/MediaFileSystems/Rt11.h>
#include <RetroMediaLab/MediaFileSystems/Ucsd.h>
#include <RetroMediaLab/MediaFileSystems/AppleDos.h>
#include <RetroMediaLab/MediaFileSystems/ProDos.h>
#include <RetroMediaLab/MediaFileSystems/MacMfs.h>
#include <RetroMediaLab/MediaFileSystems/MacHfs.h>
#include <RetroMediaLab/MediaFileSystems/Lisa.h>
#include <RetroMediaLab/MediaFileSystems/Cpm.h>
#include <RetroMediaLab/MediaFileSystems/CommodoreDos.h>
#include <RetroMediaLab/MediaFileSystems/Fat12.h>
#include <RetroMediaLab/MediaFileSystems/SpartaDos.h>
#include <RetroMediaLab/MediaFileSystems/AtariDos.h>
#include "../../FileSystems/Atari/FormatRecognition.h"
namespace RetroMediaLab::MediaFileSystems {
namespace Composition {
std::shared_ptr<const SectorReader> createAmigaDosDocumentReader();
}
namespace {
using Probe=bool(*)(const SectorImage&,std::stop_token);
using Read=FileSystemVolume(*)(const SectorImage&,std::stop_token);
class Reader final:public SectorReader {
    std::string_view id_;std::span<const std::string_view> formats_;Probe probe_;Read read_;
public:
    Reader(std::string_view id,std::span<const std::string_view> formats,Probe probe,Read read):id_(id),formats_(formats),probe_(probe),read_(read){}
    std::string_view id()const noexcept override{return id_;}
    std::span<const std::string_view> catalogFormatIds()const noexcept override{return formats_;}
    bool canRead(const SectorImage& image,std::stop_token token)const override{checkCancellation(token);return probe_(image,token);}
    FileSystemVolume read(const SectorImage& image,std::stop_token token)const override{checkCancellation(token);return read_(image,token);}
};
}
std::vector<std::shared_ptr<const SectorReader>> createDefaultSectorReaders(){
    using namespace Composition::CatalogConstants;
    return {
        Composition::createAmigaDosDocumentReader(),
        std::make_shared<Reader>(Reader1,Formats1,Probe(AmigaFlatArchive::canRead),Read(AmigaFlatArchive::read)),
        Composition::createAdfsDocumentReader(),
        std::make_shared<Reader>(Reader3,Formats3,Probe(BbcDfs::canRead),Read(BbcDfs::read)),
        std::make_shared<Reader>(Reader4,Formats4,Probe([](const SectorImage& image,std::stop_token token){checkCancellation(token);return Coherent::canRead(image);}),Read(Coherent::read)),
        std::make_shared<Reader>(Reader5,Formats5,Probe(Rt11::canRead),Read(Rt11::read)),
        std::make_shared<Reader>(Reader6,Formats6,Probe(Ucsd::canRead),Read(Ucsd::read)),
        std::make_shared<Reader>(Reader7,Formats7,Probe(AppleDos::canRead),Read(AppleDos::read)),
        std::make_shared<Reader>(Reader8,Formats8,Probe(ProDos::canRead),Read(ProDos::read)),
        std::make_shared<Reader>(Reader9,Formats9,Probe(MacMfs::canRead),Read(MacMfs::read)),
        std::make_shared<Reader>(Reader10,Formats10,Probe(MacHfs::canRead),Read(MacHfs::read)),
        std::make_shared<Reader>(Reader11,Formats11,Probe(Lisa::canRead),Read(Lisa::read)),
        std::make_shared<Reader>(Reader12,Cpm::amstradCatalogFormatIds(),Probe(Cpm::canReadAmstrad),Read(Cpm::readAmstrad)),
        std::make_shared<Reader>(Reader13,Cpm::catalogFormatIds(),Probe(Cpm::canRead),Read(Cpm::read)),
        std::make_shared<Reader>(Reader14,Formats14,Probe(CommodoreDos::canRead),Read(CommodoreDos::read)),
        std::make_shared<Reader>(Reader15,Formats15,Probe(Fat12::canRead),Read(Fat12::read)),
        std::make_shared<Reader>(Reader16,Formats16,Probe(SpartaDos::canRead),Read(SpartaDos::read)),
        std::make_shared<Reader>(Reader17,Formats17,Probe(AtariDos::canRead),Read(AtariDos::read)),
        std::make_shared<Reader>(Reader18,AtariMetadata::Formats,Probe(AtariMetadata::kFile),Read(AtariMetadata::readKFile)),
        std::make_shared<Reader>(Reader19,AtariMetadata::Formats,Probe(AtariMetadata::clk),Read(AtariMetadata::readClk)),
        std::make_shared<Reader>(Reader20,AtariMetadata::Formats,Probe(AtariMetadata::bootDisk),Read(AtariMetadata::readBootDisk)),
    };
}
}
