#include "ExplorationMapper.h"
#include "Constants.h"
namespace RetroMediaLab::MediaEngine::ExplorationMapper {
namespace M=MediaFileSystems;
MessageInfo message(const M::MessageRecipe& value){return {value.resourceKey,value.arguments,value.translationDomain};}
VolumeInfo volumeDescriptor(const M::VolumeDescriptor& value){return {std::int64_t(value.start()),std::int64_t(value.length()),value.origin(),value.partitionTable(),value.partitionNumber(),value.sessionNumber(),value.trackNumber(),value.fileSystemId(),value.partitionType(),value.partitionId(),value.name(),value.nameRecipe()?std::optional(message(*value.nameRecipe())):std::nullopt};}
DiagnosticInfo diagnostic(const M::Diagnostic& value){DiagnosticInfo result{message(M::diagnosticMessage(value)),value.context,{}};for(const auto& cause:value.causes)result.causes.push_back(diagnostic(cause));return result;}
namespace {
std::optional<FileTimestamp> timestamp(const std::optional<M::Timestamp>& value){if(!value)return {};return FileTimestamp{value->year,value->month,value->day,value->hour,value->minute,value->second,value->nanosecond,value->offsetMinutes};}
FileEntryAnalysis analysis(const M::EntryAnalysis& value){return {value.categoryId,value.iconId,value.typeResourceKey,value.extension,value.executionKindId,value.contentFormatId,value.textEncodingId,value.previewKindId,value.typeTranslationDomain};}
FileSystemEntryInfo entry(const M::FileSystemEntry& value,const std::shared_ptr<const void>& owner,const M::AnalyzedFileSystemEntry* analyzed){
    FileSystemEntryInfo result;
    switch(value.kind){case M::EntryKind::Directory:result.kind=FileSystemEntryKind::Directory;break;case M::EntryKind::File:result.kind=FileSystemEntryKind::File;break;case M::EntryKind::Link:result.kind=FileSystemEntryKind::Link;break;default:break;}
    result.name=value.name;
    result.size=value.size;
    result.comment=value.comment;
    result.rawAttributes=value.rawAttributes;
    result.storageReference=value.storageReference;
    result.metadataValid=value.metadataValid;
    result.nativeTypeId=value.nativeTypeId;
    result.occupiedSize=value.occupiedSize;
    result.attributes=value.attributes;
    result.dataValid=value.dataValid;
    result.syntheticName=value.syntheticName;
    result.linkTarget=value.linkTarget;
    result.commentResourceKey=value.commentResourceKey;
    result.commentTranslationDomain=value.commentTranslationDomain;
    result.commentArguments=value.commentArguments;
    result.metadata=value.metadata;

    result.modified=timestamp(value.modified);result.created=timestamp(value.created);result.accessed=timestamp(value.accessed);
    if(value.content)result.content=std::shared_ptr<const std::vector<std::uint8_t>>(owner,&*value.content);
    for(const auto& error:value.diagnostics)result.diagnostics.push_back(diagnostic(error));
    if(analyzed){result.analysis=analysis(analyzed->analysis);for(const auto& child:analyzed->children)if(child.entry)result.children.push_back(entry(*child.entry,child.entry,&child));}
    else {if(value.analysis)result.analysis=analysis(*value.analysis);for(const auto& child:value.children)result.children.push_back(entry(child,owner,nullptr));}
    return result;
}
M::MessageRecipe sourceMessage(const MessageInfo& value){return {value.resourceKey,value.arguments,value.translationDomain};}
M::Diagnostic sourceDiagnostic(const DiagnosticInfo& value){
    M::Diagnostic result{M::DiagnosticCode::UnknownDiagnostic,value.message.arguments,value.context,{},sourceMessage(value.message)};
    for(const auto& cause:value.causes)result.causes.push_back(sourceDiagnostic(cause));
    return result;
}
std::optional<M::Timestamp> sourceTimestamp(const std::optional<FileTimestamp>& value){if(!value)return {};return M::Timestamp{value->year,value->month,value->day,value->hour,value->minute,value->second,value->nanosecond,value->offsetMinutes};}
M::FileSystemEntry sourceEntry(const FileSystemEntryInfo& value){
    M::FileSystemEntry result;
    switch(value.kind){case FileSystemEntryKind::Directory:result.kind=M::EntryKind::Directory;break;case FileSystemEntryKind::File:result.kind=M::EntryKind::File;break;case FileSystemEntryKind::Link:result.kind=M::EntryKind::Link;break;default:break;}
    result.name=value.name;result.size=value.size;result.comment=value.comment;result.rawAttributes=value.rawAttributes;result.storageReference=value.storageReference;result.metadataValid=value.metadataValid;
    result.nativeTypeId=value.nativeTypeId;result.occupiedSize=value.occupiedSize;result.attributes=value.attributes;result.dataValid=value.dataValid;result.syntheticName=value.syntheticName;result.linkTarget=value.linkTarget;
    result.commentResourceKey=value.commentResourceKey;result.commentTranslationDomain=value.commentTranslationDomain;result.commentArguments=value.commentArguments;result.metadata=value.metadata;
    result.modified=sourceTimestamp(value.modified);result.created=sourceTimestamp(value.created);result.accessed=sourceTimestamp(value.accessed);
    if(value.content)result.content=*value.content;
    for(const auto& diagnostic:value.diagnostics)result.diagnostics.push_back(sourceDiagnostic(diagnostic));
    if(value.analysis){const auto& a=*value.analysis;result.analysis=M::EntryAnalysis{a.categoryId,a.iconId,a.typeResourceKey,a.extension,a.executionKindId,a.contentFormatId,a.textEncodingId,a.previewKindId,a.typeTranslationDomain};}
    for(const auto& child:value.children)result.children.push_back(sourceEntry(child));
    return result;
}
}
M::FileSystemVolume toFileSystemsVolume(const FileSystemVolumeInfo& value){
    M::FileSystemVolume result;
    result.name=value.name;result.fileSystemId=value.fileSystemId;result.fileSystemDisplayName=value.fileSystemDisplayName;
    result.fileSystemDisplayResourceKey=value.fileSystemDisplayResourceKey;result.fileSystemDisplayTranslationDomain=value.fileSystemDisplayTranslationDomain;result.fileSystemDisplayArguments=value.fileSystemDisplayArguments;
    result.sourceDiagnostics=value.sourceDiagnostics;result.capacity=value.capacity;result.freeBytes=value.freeBytes;result.freeSpaceKnown=value.freeSpaceKnown;
    result.attributes=value.attributes;result.bootable=value.bootable;result.diskNumber=value.diskNumber;result.diskCount=value.diskCount;result.diskNumberOrigin=value.diskNumberOrigin;
    result.created=sourceTimestamp(value.created);result.modified=sourceTimestamp(value.modified);
    if(value.nameRecipe)result.nameRecipe=sourceMessage(*value.nameRecipe);
    for(const auto& message:value.sourceMessages)result.sourceMessages.push_back(sourceMessage(message));
    for(const auto& warning:value.warnings)result.warnings.push_back(sourceDiagnostic(warning));
    for(const auto& entry:value.entries)result.entries.push_back(sourceEntry(entry));
    return result;
}
std::shared_ptr<const FileSystemVolumeInfo> volume(std::shared_ptr<const M::FileSystemVolume> value,std::optional<std::span<const M::AnalyzedFileSystemEntry>> analyzed){
    if(!value)throw M::MediaError(M::DiagnosticCode::DocumentNull);
    auto result=std::make_shared<FileSystemVolumeInfo>();
    result->name=value->name;
    result->fileSystemId=value->fileSystemId;
    result->fileSystemDisplayName=value->fileSystemDisplayName;
    result->fileSystemDisplayResourceKey=value->fileSystemDisplayResourceKey;
    result->fileSystemDisplayTranslationDomain=value->fileSystemDisplayTranslationDomain;
    result->sourceDiagnostics=value->sourceDiagnostics;
    result->fileSystemDisplayArguments=value->fileSystemDisplayArguments;
    result->capacity=value->capacity;
    result->freeBytes=value->freeBytes;
    result->freeSpaceKnown=value->freeSpaceKnown;
    result->attributes=value->attributes;
    result->bootable=value->bootable;
    result->diskNumber=value->diskNumber;
    result->diskCount=value->diskCount;
    result->diskNumberOrigin=value->diskNumberOrigin;

    if(result->fileSystemDisplayName.empty())result->fileSystemDisplayName=result->fileSystemId;
    if(value->nameRecipe)result->nameRecipe=message(*value->nameRecipe);
    for(const auto& m:value->sourceMessages)result->sourceMessages.push_back(message(m));
    result->created=timestamp(value->created);result->modified=timestamp(value->modified);
    for(const auto& warning:value->warnings)result->warnings.push_back(diagnostic(warning));
    if(!analyzed)for(const auto& item:value->entries)result->entries.push_back(entry(item,value,nullptr));
    else for(const auto& item:*analyzed)if(item.entry)result->entries.push_back(entry(*item.entry,item.entry,&item));
    return result;
}
}
