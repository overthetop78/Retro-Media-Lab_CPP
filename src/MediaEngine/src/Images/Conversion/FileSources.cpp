#include "Files.h"
#include "FileConstants.h"
#include "Service.h"
#include "../DocumentFactory.h"
#include "../../Models/Sectors/Representation.h"
#include "../../Models/Sectors/SectorImages.h"
#include "../../Reconstruction/Iso/Reader.h"
#include "../../Formats/Raw/Adapters.h"
#include "../../Formats/Raw/SpecializedReaders.h"
#include "../../Formats/Raw/RawImages.h"
#include "../../Formats/Adf/Adapter.h"
#include "../../Formats/Adf/AdfImages.h"
#include "../../Formats/BbcDfs/Adapter.h"
#include "../../Formats/BbcDfs/BbcDfsImages.h"
#include "../../Formats/Epson/RawReader.h"
#include "../../Formats/Epson/Geometry.h"
#include "../../Formats/ImageDisk/Adapter.h"
#include "../../Formats/TeleDisk/Reader.h"
#include "../../Formats/Rx02/Adapter.h"
#include "../../Formats/CommodoreDisks/Adapters.h"
#include "../../Formats/FormatIds.h"
#include "../../Formats/Atari/Adapters.h"
#include "../../Formats/Commodore900/Constants.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::SectorFileConversion {
namespace {
std::string utf8(const std::filesystem::path& path){const auto value=path.u8string();return {value.begin(),value.end()};}
std::shared_ptr<const MediaDocument> readDocument(const ImageReader& reader,const std::filesystem::path& path,std::optional<std::string> format,std::stop_token token){SourceOptions options;options.primaryPath=utf8(path);options.requestedFormatId=std::move(format);return reader.readAsync(RecognitionContext(std::make_shared<MediaSource>(std::move(options))),token).get();}
std::shared_ptr<const MediaDocument> direct(Family family,const std::filesystem::path& path,std::string_view target,std::stop_token token){
 switch(family){
 case Family::AtariSt:return readDocument(Atari::Reader(Primitives::asciiIdentifierEqualsIgnoreCase(utf8(path.extension()),Constants::Msa)?Atari::Kind::Msa:Atari::Kind::St),path,{},token);
 case Family::Atr:return readDocument(Atari::Reader(Atari::Kind::Atr),path,{},token);
 case Family::CommodoreDos:return readDocument(CommodoreDisks::ImageReaderAdapter(Primitives::asciiIdentifierEqualsIgnoreCase(utf8(path.extension()),Constants::D71)?CommodoreDisks::Kind::D71:CommodoreDisks::Kind::D64),path,{},token);
 case Family::Coherent:return readDocument(Raw::SpecializedReader(Raw::SpecializedKind::Coherent),path,{},token);
 case Family::Ibm:return readDocument(Raw::Reader(Raw::ReaderKind::Ibm),path,{},token);
 case Family::Msx:return readDocument(Raw::Reader(Raw::ReaderKind::Msx),path,{},token);
 case Family::BbcDfs:return readDocument(BbcDfs::Reader(),path,{},token);
 case Family::AcornAdf:case Family::AmigaAdf:return readDocument(Adf::Reader(),path,{},token);
 case Family::Epson:return Primitives::asciiIdentifierEqualsIgnoreCase(utf8(path.extension()),Constants::Imd)?readDocument(ImageDisk::Reader(),path,{},token):readDocument(Epson::RawReader(),path,std::string(target),token);
 case Family::Ucsd:return Primitives::asciiIdentifierEqualsIgnoreCase(utf8(path.extension()),Constants::Td0)?readDocument(TeleDisk::Reader(),path,{},token):readDocument(Raw::SpecializedReader(Raw::SpecializedKind::Ucsd),path,{},token);
 case Family::Rx02:return readDocument(Rx02::Reader(),path,{},token);
 case Family::D81:return readDocument(CommodoreDisks::ImageReaderAdapter(CommodoreDisks::Kind::D81),path,{},token);
 default:throw MediaError(DiagnosticCode::ConversionIncompatibleFormat,{std::string(target),utf8(path.extension())});
 }
}
}
std::optional<Family> family(std::string_view id){
 const auto same=Primitives::asciiIdentifierEqualsIgnoreCase;
 if(Primitives::asciiIdentifierStartsWithIgnoreCase(id,Formats::AtariStPrefix))return Family::AtariSt;
 if(Primitives::asciiIdentifierStartsWithIgnoreCase(id,Formats::AtariPrefix))return Family::Atr;
 if(Primitives::asciiIdentifierStartsWithIgnoreCase(id,Formats::AppleLisaPrefix))return Family::Lisa;
 if(Primitives::asciiIdentifierStartsWithIgnoreCase(id,Formats::AppleMacPrefix)||Primitives::asciiIdentifierStartsWithIgnoreCase(id,Formats::MacPrefix))return Family::Macintosh;
 if(Primitives::asciiIdentifierStartsWithIgnoreCase(id,Formats::AppleIIPrefix)||Primitives::asciiIdentifierStartsWithIgnoreCase(id,Formats::AppleIIIPrefix))return Family::AppleSector;
 if(same(id,Formats::Commodore1541)||same(id,Formats::Commodore1571))return Family::CommodoreDos;
 if(same(id,Commodore900::Constants::FormatId))return Family::Coherent;
 if(same(id,Formats::AmigaDos)||same(id,Formats::AmigaDosHighDensity))return Family::AmigaAdf;
 if(BbcDfs::findGeometry(id))return Family::BbcDfs;
 if(Adf::findGeometry(id))return Family::AcornAdf;
 if(same(id,Formats::AmstradCpc)||same(id,Formats::AmstradPcw))return Family::Amstrad;
 if(std::any_of(Epson::geometries().begin(),Epson::geometries().end(),[&](const auto& g){return same(g.formatId,id);}))return Family::Epson;
 if(same(id,Formats::UcsdIbmMfm))return Family::Ucsd;
 if(same(id,Formats::DecRx02))return Family::Rx02;
 if(same(id,Formats::Commodore1581))return Family::D81;
 const auto ibm=Raw::ibmGeometries(),msx=Raw::msxGeometries();
 if(same(id,Formats::IbmScan)||std::any_of(ibm.begin(),ibm.end(),[&](const auto& g){return same(g.formatId,id);}))return Family::Ibm;
 if(std::any_of(msx.begin(),msx.end(),[&](const auto& g){return same(g.formatId,id);}))return Family::Msx;
 return {};
}
std::shared_ptr<const MediaDocument> Service::read(const std::filesystem::path& path,std::string_view target,Family family,std::stop_token token)const{
 if(family==Family::AppleSector||family==Family::AppleNibble||family==Family::Macintosh||family==Family::Lisa)return readApple(path,target,family,token);
 const bool isScp=Primitives::asciiIdentifierEqualsIgnoreCase(utf8(path.extension()),Scp::Constants::Extension);
 if(family==Family::Epson&&!isScp)return direct(family,path,target,token);
 SourceOptions options;options.primaryPath=utf8(path);const auto owner=std::make_shared<MediaSource>(std::move(options));
 const auto image=MediaConversionService::readSectorSource(path,
  [&]{return std::async(std::launch::deferred,[&]{if(family==Family::AmigaAdf)return scp_.readAmiga(path,{},token);if(family==Family::Rx02)return scp_.readRx02(path,token);
   if(family==Family::D81)return scp_.readCommodore(path,std::string(Formats::Commodore1581),token);
   if(family==Family::Coherent)return scp_.readCommodore(path,std::string(Commodore900::Constants::FormatId),token);
   if(family==Family::CommodoreDos)return scp_.readCommodore(path,std::string(Primitives::asciiIdentifierEqualsIgnoreCase(target,Formats::Commodore1571)?Formats::Commodore1571:Formats::Commodore1541),token);
   if(family==Family::AtariSt||family==Family::Atr)return scp_.isoReader().readAtari(path,std::string(target),{},token);return scp_.isoReader().read(path,std::string(target),{},token);});},
  [&]{return std::async(std::launch::deferred,[&]{const auto document=direct(family,path,target,token);const auto sectors=dynamic_cast<const SectorRepresentation*>(&document->representation());if(!sectors)throw MediaError(DiagnosticCode::ReaderNullDocument);return sectors->imageOwner();});}).get();
 return floppySectorDocument(owner,family==Family::Ucsd?relabelSectorImage(image,std::string(Formats::UcsdIbmMfm)):image);
}
}
