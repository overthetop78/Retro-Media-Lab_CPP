#include "Internal.h"
#include <algorithm>
namespace RetroMediaLab::MediaFileSystems::MacHfs::Internal {
using namespace Constants;
namespace {
std::vector<FileSystemEntry> children(std::uint32_t parent,const std::vector<Record>& records,std::set<std::uint32_t> path,std::vector<Diagnostic>& warnings,std::stop_token token){
    checkCancellation(token);if(!path.insert(parent).second){warnings.push_back({DiagnosticCode::MacDirectoryCycle,{std::to_string(parent)}});return {};}
    std::vector<FileSystemEntry> result;
    for(const auto& value:records){checkCancellation(token);if(value.parent!=parent)continue;FileSystemEntry entry;entry.name=value.name;entry.kind=value.directory?EntryKind::Directory:EntryKind::File;entry.size=value.size;entry.modified=value.modified;entry.comment=value.type;entry.storageReference=Macintosh::storageReference(value.id);entry.metadataValid=value.valid;
        if(value.directory){entry.children=children(value.id,records,path,warnings,token);entry.commentResourceKey=DirectoryKey;entry.commentTranslationDomain=TranslationDomain;}
        else{entry.content=value.data.empty()?value.resources:value.data;if(value.defaultType){entry.commentResourceKey=DefaultFileKey;entry.commentTranslationDomain=TranslationDomain;}}
        result.push_back(std::move(entry));
    }
    std::stable_sort(result.begin(),result.end(),[](const auto& a,const auto& b){if(a.kind!=b.kind)return a.kind==EntryKind::Directory;return Macintosh::lessName(a.name,b.name);});return result;
}
}
std::vector<FileSystemEntry> buildDirectory(const std::vector<Record>& records,std::vector<Diagnostic>& warnings,std::stop_token token){return children(RootDirectoryId,records,{},warnings,token);}
}
