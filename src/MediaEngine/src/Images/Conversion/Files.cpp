#include "Files.h"
#include "FileConstants.h"
#include "../DocumentFactory.h"
#include "../../Composition/Runtime.h"
#include "../../Models/Sectors/Representation.h"
#include "../../Formats/CpcDsk/CpcDsk.h"
#include "../../Formats/Adf/AdfImages.h"
#include "../../Formats/BbcDfs/BbcDfsImages.h"
#include "../../Formats/Raw/RawImages.h"
#include "../../Formats/Apple/Constants.h"
#include "../../Formats/Apple/Writing.h"
#include "../../Formats/FormatIds.h"
#include "../../Formats/St/Transform.h"
#include "../../Formats/St/Writer.h"
#include "../../Formats/Msa/Msa.h"
#include "../../Formats/CommodoreDisks/Images.h"
#include "../../Reconstruction/Iso/Reader.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::SectorFileConversion {
namespace {
std::string utf8(const std::filesystem::path& path){const auto value=path.u8string();return {value.begin(),value.end()};}
}
Service::Service(ScpReconstruction::Composition scp,MediaWritingComposition writing):scp_(std::move(scp)),writing_(std::move(writing)){}
bool Service::canCreate(std::string_view target,std::string_view extension)const{return !Primitives::isBlankIdentifier(extension)&&extension.front()==Constants::ExtensionSeparator&&family(target).has_value()&&!writing_.registry().findTargetCandidates(target,extension).empty();}
std::future<std::vector<std::filesystem::path>> Service::convertAsync(std::filesystem::path source,std::filesystem::path output,std::string target,std::stop_token token)const{
 return std::async(std::launch::async,[service=*this,source=std::move(source),output=std::move(output),target=std::move(target),token]{
  checkCancellation(token);auto selected=family(target);const auto extension=utf8(output.extension());
  if(!selected||!service.canCreate(target,extension))throw MediaError(DiagnosticCode::ImageWriteUnavailable,{target,extension});
  if(selected==Family::AppleSector&&Apple::DiskImageWriter::supportsExtension(extension))selected=Family::AppleNibble;
  std::error_code error;if(std::filesystem::equivalent(source,output,error))throw MediaError(DiagnosticCode::AtomicSetExists,{utf8(output)});
  if(*selected==Family::Amstrad){
   const auto kind=Primitives::asciiIdentifierEqualsIgnoreCase(extension,Constants::Edsk)?CpcDsk::ContainerKind::Extended:CpcDsk::ContainerKind::Standard;
   auto container=Primitives::asciiIdentifierEqualsIgnoreCase(utf8(source.extension()),Scp::Constants::Extension)?CpcDsk::buildImage(service.scp_.isoReader().read(source,target,{},token),kind,token):CpcDsk::readDetailed(source,token);
   container.kind=kind;CpcDsk::write(container,output,token);return std::vector<std::filesystem::path>{output};
  }
  auto document=service.read(source,target,*selected,token);auto sectors=dynamic_cast<const SectorRepresentation*>(&document->representation());if(!sectors)throw MediaError(DiagnosticCode::ReaderNullDocument);
  if(*selected==Family::Epson&&!Primitives::asciiIdentifierEqualsIgnoreCase(sectors->image().geometry().formatId,target))throw MediaError(DiagnosticCode::EpsonWriteSourceFormat,{sectors->image().geometry().formatId,target});
  if(*selected==Family::AmigaAdf&&!Primitives::asciiIdentifierEqualsIgnoreCase(sectors->image().geometry().formatId,target))throw MediaError(DiagnosticCode::ScpPreparationFormat,{sectors->image().geometry().formatId,target});
  if(*selected==Family::Lisa&&!Primitives::asciiIdentifierStartsWithIgnoreCase(sectors->image().geometry().formatId,Formats::AppleLisaPrefix))throw MediaError(DiagnosticCode::ScpPreparationFormat,{sectors->image().geometry().formatId,target});
  if(*selected==Family::AtariSt){auto transformed=St::transform(sectors->image(),target,Primitives::asciiIdentifierEqualsIgnoreCase(utf8(source.extension()),Scp::Constants::Extension),token);document=floppySectorDocument(document->mediaSource(),std::move(transformed));sectors=dynamic_cast<const SectorRepresentation*>(&document->representation());}
  if(*selected==Family::CommodoreDos){
   if(!Primitives::asciiIdentifierEqualsIgnoreCase(sectors->image().geometry().formatId,target))throw MediaError(DiagnosticCode::ScpPreparationFormat,{sectors->image().geometry().formatId,target});
   bool preserve=true;sectors->image().visitAvailableBlocks([&](auto number){checkCancellation(token);preserve=sectors->image().block(number,token)->diagnosticCode.has_value();return preserve;},token);
   CommodoreDisks::writeD64D71(sectors->image(),output,preserve?CommodoreDisks::ErrorMapMode::Preserve:CommodoreDisks::ErrorMapMode::None,token);return std::vector<std::filesystem::path>{output};
  }
  switch(*selected){
  case Family::AtariSt:if(Primitives::asciiIdentifierEqualsIgnoreCase(extension,Constants::Msa))Msa::write(sectors->image(),output,token);else St::write(sectors->image(),output,token);break;
  case Family::AcornAdf:case Family::AmigaAdf:Adf::write(sectors->image(),output,token);break;
  case Family::BbcDfs:BbcDfs::write(sectors->image(),output,target,token);break;
  case Family::Ibm:Raw::writeIbm(sectors->image(),output,target,token);break;
  case Family::Msx:Raw::writeMsx(sectors->image(),output,target,token);break;
  default:return service.writing_.writingService().writeAsync(document,output,target,token).get();
  }
  return std::vector<std::filesystem::path>{output};
 });
}
}
namespace RetroMediaLab::MediaEngine {
bool isSectorFileCatalogAliasTarget(std::string_view format,std::string_view extension){const auto same=Primitives::asciiIdentifierEqualsIgnoreCase;return Apple::DiskImageWriter::supportsExtension(extension)&&(same(format,Formats::AppleIIAppleDos113)||same(format,Formats::AppleIIAppleDos140)||same(format,Formats::AppleIIProDos140));}
bool canConvertSectorFileTarget(std::string_view format,std::string_view extension){return RuntimeDetails::defaultServices()->sectorFiles.canCreate(format,extension);}
std::future<std::vector<std::filesystem::path>> convertSectorFileAsync(std::filesystem::path source,std::filesystem::path output,std::string target,std::stop_token token){return RuntimeDetails::defaultServices()->sectorFiles.convertAsync(std::move(source),std::move(output),std::move(target),token);}
}
