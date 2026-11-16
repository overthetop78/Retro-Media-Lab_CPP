#include <RetroMediaLab/MediaEngine/Migration.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaFileSystems/Migration.h>
#include "Blank.h"
#include "Constants.h"
#include "../MediaDocument.h"
#include "../../Models/Sectors/Representation.h"
#include "../../Primitives/Identifiers.h"
#include "../../Formats/Adf/AdfImages.h"
#include "../../Formats/Raw/RawImages.h"
#include "../../Formats/St/Geometry.h"
#include "../../Formats/St/Writer.h"
#include "../../Formats/Apple/Writing.h"
#include "../../Formats/Apple/Raw.h"
#include "../../Formats/TwoImg/TwoImg.h"
#include "../../Formats/CommodoreDisks/Images.h"
namespace RetroMediaLab::MediaEngine {
namespace {
namespace M = MediaFileSystems;
// Project only the fields consumed by MigrationPlanner; exploration metadata is not rewritten.
M::FileSystemEntry migrationEntry(const FileSystemEntryInfo& source) {
    M::FileSystemEntry result;result.name=source.name;result.size=source.size;
    switch(source.kind){case FileSystemEntryKind::Directory:result.kind=M::EntryKind::Directory;break;case FileSystemEntryKind::File:result.kind=M::EntryKind::File;break;case FileSystemEntryKind::Link:result.kind=M::EntryKind::Link;break;default:result.kind=M::EntryKind::Unknown;break;}
    if(source.content)result.content=*source.content;
    if(source.modified){const auto& t=*source.modified;result.modified=M::Timestamp{t.year,t.month,t.day,t.hour,t.minute,t.second,t.nanosecond,t.offsetMinutes};}
    result.comment=source.comment;result.rawAttributes=source.rawAttributes;result.metadataValid=source.metadataValid;
    for(const auto& child:source.children)result.children.push_back(migrationEntry(child));
    return result;
}
M::MigrationPlan plan(const FileSystemVolumeInfo& source,const FileSystemMigrationTarget& target) {
    M::FileSystemVolume volume;volume.name=source.name;volume.fileSystemId=source.fileSystemId;
    for(const auto& entry:source.entries)volume.entries.push_back(migrationEntry(entry));
    return M::FileSystemMigrationService::createPlan(volume,std::string(target.fileSystemId));
}
MigrationLossKind lossKind(M::MigrationLossKind value) {
    switch(value){
    case M::MigrationLossKind::MissingContent:return MigrationLossKind::MissingContent;
    case M::MigrationLossKind::UnsupportedEntryKind:return MigrationLossKind::UnsupportedEntryKind;
    case M::MigrationLossKind::InvalidName:return MigrationLossKind::InvalidName;
    case M::MigrationLossKind::NameTooLong:return MigrationLossKind::NameTooLong;
    case M::MigrationLossKind::NameCollision:return MigrationLossKind::NameCollision;
    case M::MigrationLossKind::FileTooLarge:return MigrationLossKind::FileTooLarge;
    case M::MigrationLossKind::InvalidMetadata:return MigrationLossKind::InvalidMetadata;
    case M::MigrationLossKind::ModifiedDate:return MigrationLossKind::ModifiedDate;
    case M::MigrationLossKind::Comment:return MigrationLossKind::Comment;
    case M::MigrationLossKind::Attributes:return MigrationLossKind::Attributes;
    }
    return MigrationLossKind::InvalidMetadata;
}
MigrationValidationReport report(const M::MigrationValidationReport& source) {
    MigrationValidationReport result{{},source.metadataLossAccepted};
    for(const auto& loss:source.losses)result.losses.push_back({lossKind(loss.kind),loss.path,loss.isBlocking,loss.detail});
    return result;
}
void writeImage(const M::SectorImage& image,const std::filesystem::path& output,std::string_view format,std::stop_token token) {
    using namespace Formats;
    const auto suffix=output.extension().u8string();const std::string extension(suffix.begin(),suffix.end());
    const auto same=[](std::string_view a,std::string_view b){return Primitives::asciiIdentifierEqualsIgnoreCase(a,b);};
    if(format.starts_with(AmigaPrefix)){Adf::write(image,output,token);return;}
    if(St::geometry(format)){St::write(image,output,token);return;}
    if(format.starts_with(IbmPrefix)){Raw::writeIbm(image,output,format,token);return;}
    if(format.starts_with(MsxPrefix)){Raw::writeMsx(image,output,format,token);return;}
    if(format.starts_with(AppleIIPrefix)||format.starts_with(AppleIIIPrefix)){
        if(same(extension,Apple::Constants::TwoMg))TwoImg::write(image,output,format,token);
        else if(Apple::DiskImageWriter::supportsExtension(extension)&&format!=AppleIIProDos800&&format!=AppleIIISos)Apple::DiskImageWriter{}.write(image,output,token);
        else if(Apple::sectorTarget(format,extension))Apple::writeRaw(image,output,format,token);
        else throw MediaError(DiagnosticCode::MigrationAppleTarget,{std::string(format),extension});
        return;
    }
    if((format==Commodore1541&&same(extension,CommodoreDisks::Constants::D64Extension))||(format==Commodore1571&&same(extension,CommodoreDisks::Constants::D71Extension)))
        CommodoreDisks::writeD64D71(image,output,CommodoreDisks::ErrorMapMode::None,token);
    else if(format==Commodore1581&&same(extension,CommodoreDisks::Constants::D81Extension))CommodoreDisks::writeD81(image,output,token);
    else throw MediaError(DiagnosticCode::MigrationCommodoreTarget,{std::string(format),extension});
}
}
MigrationValidationReport FileSystemMigrationService::validate(const FileSystemVolumeInfo& source,std::string_view format,bool accept) {
    const auto& target=FileSystemMigrationTargetCatalog::get(format);
    const auto blank=BlankSectorImageFactory::create(target.formatId);
    return report(M::FileSystemMigrationService::validate(plan(source,target),*blank,accept));
}
MigrationResult FileSystemMigrationService::createImage(const FileSystemVolumeInfo& source,std::string_view format,bool accept,std::stop_token token) {
    checkCancellation(token);const auto& target=FileSystemMigrationTargetCatalog::get(format);
    const auto blank=BlankSectorImageFactory::create(target.formatId,token);
    const auto injected=M::FileSystemMigrationService::inject(plan(source,target),*blank,accept,token);
    const auto image=imageFromWritePlan(injected.image);
    SourceOptions options;options.primaryPath=CreationConstants::GeneratedSource;options.knownLength=std::int64_t(image->capacity());options.requestedFormatId=std::string(target.formatId);
    auto document=std::make_shared<MediaDocument>(std::make_shared<MediaSource>(std::move(options)),std::string(target.formatId),MediaKind::Floppy,std::make_shared<SectorRepresentation>(image));
    return {std::move(document),report(injected.report)};
}
std::future<MigrationResult> FileSystemMigrationService::writeAsync(FileSystemVolumeInfo source,std::filesystem::path output,std::string format,bool accept,std::stop_token token) {
    return std::async(std::launch::async,[source=std::move(source),output=std::move(output),format=std::move(format),accept,token]{
        checkCancellation(token);auto result=createImage(source,format,accept,token);
        const auto& sectors=dynamic_cast<const M::SectorRepresentation&>(result.image->representation());
        writeImage(sectors.image(),output,result.image->formatId(),token);return result;
    });
}
}
