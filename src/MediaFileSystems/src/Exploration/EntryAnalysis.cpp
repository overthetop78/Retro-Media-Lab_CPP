#include <RetroMediaLab/MediaFileSystems/FileSystemModels.h>
#include <RetroMediaLab/MediaAnalysis/Classifier.h>
#include "AnalysisConstants.h"
#include "../Primitives/Identifiers.h"
#include "../FileSystems/Commodore/Dos/Constants.h"
#include <algorithm>
#include "../Primitives/MetadataConstants.h"
#include <QString>
namespace RetroMediaLab::MediaFileSystems {
namespace {
namespace Analysis = RetroMediaLab::MediaAnalysis;
using namespace AnalysisConstants;
std::string asciiLower(std::string_view value) {
    std::string result(value);for(auto& character:result)if(character>=MetadataConstants::UpperFirst && character<=MetadataConstants::UpperLast)character=char(character+MetadataConstants::CaseDelta);return result;
}
Analysis::MediaFileSystemFamily family(std::string_view format,std::string_view fileSystem) {
    using enum Analysis::MediaFileSystemFamily;
    const auto lower=asciiLower(format);
    if(lower==AtariTape)return Atari8Bit;
    if(asciiLower(fileSystem).find(CpmFileSystem)!=std::string::npos)return Cpm;
    if(lower.starts_with(Prefix0))return BbcMicro;
    if(lower.starts_with(Prefix1))return Dec;
    if(lower.starts_with(Prefix2))return Msx;
    if(lower.starts_with(Prefix3))return Ucsd;
    if(lower.starts_with(Prefix4))return Amiga;
    if(lower.starts_with(Prefix5))return IbmPc;
    if(lower.starts_with(Prefix6))return AtariTos;
    if(lower.starts_with(Prefix7))return Atari8Bit;
    if(lower.starts_with(Prefix8))return AppleDos;
    if(lower.starts_with(Prefix9))return AppleDos;
    if(lower.starts_with(Prefix10))return ProDos;
    if(lower.starts_with(Prefix11))return ProDos;
    if(lower.starts_with(Prefix12))return Macintosh;
    if(lower.starts_with(Prefix13))return Macintosh;
    if(lower.starts_with(Prefix14))return Lisa;
    if(lower.starts_with(Prefix15))return Commodore;
    return Unknown;
}
std::string_view extension(std::string_view name) {
    for(auto offset=name.size();offset>0;--offset) {
        const auto value=name[offset-1];
        if(value==MetadataConstants::ExtensionSeparator)return offset==name.size()?std::string_view{}:name.substr(offset-1);
        if(value==MetadataConstants::PathSeparator || value==MetadataConstants::AlternatePathSeparator)break;
    }
    return {};
}
EntryAnalysis classifyEntry(const FileSystemEntry& entry,Analysis::MediaFileSystemFamily selectedFamily,std::stop_token token) {
    checkCancellation(token);Analysis::ClassificationInput input;
    input.extension=extension(entry.name);
    switch(entry.kind) {
    case EntryKind::Directory:input.kind=Analysis::MediaEntryKind::Directory;break;
    case EntryKind::File:input.kind=Analysis::MediaEntryKind::File;break;
    case EntryKind::Link:input.kind=Analysis::MediaEntryKind::Link;break;
    default:input.kind=Analysis::MediaEntryKind::Unknown;break;
    }
    if(entry.nativeTypeId)input.nativeTypeId=*entry.nativeTypeId;
    input.comment=entry.comment;
    // Display comments are localized recipes; classification consumes only their invariant type.
    if(entry.commentResourceKey&&entry.commentTranslationDomain==CommodoreDos::Constants::TranslationDomain&&!entry.commentArguments.empty()&&
       std::find(CommodoreDos::Constants::CommentKeys.begin(),CommodoreDos::Constants::CommentKeys.end(),*entry.commentResourceKey)!=CommodoreDos::Constants::CommentKeys.end())input.comment=entry.commentArguments.front();
    input.dataValid=entry.dataValid;if(entry.content)input.content=Analysis::Bytes(*entry.content);
    input.metadata=entry.metadata;input.family=selectedFamily;
    const auto type=Analysis::classify(input);
    return EntryAnalysis{std::string(MediaContentCategoryNames[std::size_t(type.category)]),type.iconId,type.typeResourceKey,type.extension,
        std::string(MediaExecutionKindNames[std::size_t(type.executionKind)]),std::string(MediaContentFormatNames[std::size_t(type.contentFormat)]),
        std::string(MediaTextEncodingNames[std::size_t(type.textEncoding)]),std::string(MediaPreviewKindNames[std::size_t(type.previewKind)]),std::string(Analysis::Localization::Domain)};
}
void analyze(FileSystemEntry& entry,Analysis::MediaFileSystemFamily selectedFamily,std::stop_token token) {
    entry.analysis=classifyEntry(entry,selectedFamily,token);
    for(auto& child:entry.children)analyze(child,selectedFamily,token);
}
AnalyzedFileSystemEntry view(const FileSystemEntry& entry,const std::shared_ptr<const FileSystemVolume>& owner,Analysis::MediaFileSystemFamily selectedFamily,std::stop_token token) {
    AnalyzedFileSystemEntry result{std::shared_ptr<const FileSystemEntry>(owner,&entry),classifyEntry(entry,selectedFamily,token),{}};
    result.children.reserve(entry.children.size());
    for(const auto& child:entry.children)result.children.push_back(view(child,owner,selectedFamily,token));
    return result;
}
}
void analyzeEntries(std::string_view formatId,FileSystemVolume& volume,std::stop_token token) {
    checkCancellation(token);
    if(Identifiers::isBlank(formatId))throw MediaError(DiagnosticCode::SectorInvalidFormatId,{std::string(formatId)});
    const auto selectedFamily=family(formatId,volume.fileSystemId);
    for(auto& entry:volume.entries)analyze(entry,selectedFamily,token);
}
std::vector<AnalyzedFileSystemEntry> analyzeEntryViews(std::string_view formatId,std::shared_ptr<const FileSystemVolume> volume,std::stop_token token) {
    checkCancellation(token);
    if(!volume)throw MediaError(DiagnosticCode::FileSystemVolumeNull);
    if(Identifiers::isBlank(formatId))throw MediaError(DiagnosticCode::SectorInvalidFormatId,{std::string(formatId)});
    const auto selectedFamily=family(formatId,volume->fileSystemId);
    std::vector<AnalyzedFileSystemEntry> result;result.reserve(volume->entries.size());
    for(const auto& entry:volume->entries)result.push_back(view(entry,volume,selectedFamily,token));
    return result;
}
}
