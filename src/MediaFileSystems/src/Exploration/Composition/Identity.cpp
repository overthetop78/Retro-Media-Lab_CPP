#include "Internal.h"
#include "Constants.h"
#include <QString>
#include <QList>
#include <QChar>
#include <algorithm>
namespace RetroMediaLab::MediaFileSystems::Composition {
std::u16string ordinalKey(std::string_view value){
    const auto scalars=QString::fromUtf8(value.data(),qsizetype(value.size())).toUcs4();std::u16string key;
    for(char32_t scalar:scalars){const auto upper=QChar::toUpper(scalar);if(scalar<=AsciiMaximum||upper>AsciiMaximum)scalar=upper;
        if(scalar<SurrogateOffset)key.push_back(char16_t(scalar));else{scalar-=SurrogateOffset;key.push_back(char16_t(HighSurrogate+(scalar>>SurrogateShift)));key.push_back(char16_t(LowSurrogate+(scalar&SurrogateMask)));}}
    return key;
}
bool blank(std::string_view value){const auto text=QString::fromUtf8(value.data(),qsizetype(value.size()));return text.isEmpty()||std::all_of(text.begin(),text.end(),[](QChar c){return c.isSpace();});}
namespace {
std::string kind(EntryKind value){switch(value){case EntryKind::Directory:return std::string(DirectoryKind);case EntryKind::File:return std::string(FileKind);case EntryKind::Link:return std::string(LinkKind);case EntryKind::Unknown:return std::string(UnknownKind);default:return std::to_string(int(value));}}
void appendEntries(std::string& target,const std::vector<FileSystemEntry>& entries,const std::string& prefix,bool& first){
    std::vector<const FileSystemEntry*> ordered;for(const auto& entry:entries)ordered.push_back(&entry);
    std::stable_sort(ordered.begin(),ordered.end(),[](auto a,auto b){return ordinalKey(a->name)<ordinalKey(b->name);});
    for(const auto* entry:ordered){if(!first)target+=EntrySeparator;first=false;const auto path=prefix+entry->name;target+=path;target+=Null;target+=kind(entry->kind);target+=Null;target+=std::to_string(entry->size);appendEntries(target,entry->children,path+PathSeparator,first);}
}
}
std::u16string identity(std::string_view format,const FileSystemVolume& volume){std::string result(format.substr(0,format.find(FormatSeparator)));result+=Null;result+=volume.name;result+=Null;bool first=true;appendEntries(result,volume.entries,{},first);return ordinalKey(result);}
}
