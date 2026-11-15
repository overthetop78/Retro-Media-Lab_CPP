#include "Internal.h"
#include <QString>
#include <algorithm>
namespace RetroMediaLab::MediaFileSystems::Coherent::Internal {
DirectoryData directory(Image& image,std::uint32_t zoneEnd,std::uint16_t number,std::set<std::uint16_t>& stack,std::set<std::uint16_t>& visited,std::vector<Diagnostic>& warnings,const std::string& name,std::uint32_t depth){
 checkCancellation(image.token);
 if(stack.contains(number)){warnings.push_back({DiagnosticCode::CoherentDirectoryCycle,{name,std::to_string(number)}});return {{},false};}
 if(!visited.insert(number).second){warnings.push_back({DiagnosticCode::CoherentDirectoryRepeated,{name,std::to_string(number)}});return {{},false};}
 if(depth>MaximumDepth){warnings.push_back({DiagnosticCode::CoherentDepth,{name,std::to_string(depth)}});return {{},false};}
 stack.insert(number);const auto current=inode(image,zoneEnd,number);const auto data=file(image,current,warnings,name);DirectoryData result;result.valid=data.valid;
 for(std::size_t offset=0;offset+DirectoryEntrySize<=data.content.size();offset+=DirectoryEntrySize){
  checkCancellation(image.token);const auto childNumber=Binary::little<std::uint16_t>(data.content,offset);if(!childNumber)continue;
  const auto childName=decodeName(Bytes(data.content).subspan(offset+DirectoryInodeLength,DirectoryNameLength));if(childName.empty()||childName==Current||childName==Parent)continue;
  try{
   const auto child=inode(image,zoneEnd,childNumber);FileSystemEntry entry;entry.name=childName;entry.kind=entryKind(child.mode);bool valid=data.valid;
   if(entry.kind==EntryKind::Directory){auto nested=directory(image,zoneEnd,childNumber,stack,visited,warnings,childName,depth+1);entry.children=std::move(nested.entries);valid=valid&&nested.valid;}
   else{auto content=file(image,child,warnings,childName);entry.content=std::move(content.content);valid=valid&&content.valid;}
   entry.size=child.size;entry.modified=decodeTime(child.modified);entry.comment=inodeDescription(childNumber);entry.rawAttributes=child.mode&ProtectionMask;entry.storageReference=childNumber;entry.metadataValid=valid;
   entry.commentResourceKey=InodeCommentKey;entry.commentTranslationDomain=TranslationDomain;entry.commentArguments={std::to_string(childNumber)};
   result.entries.push_back(std::move(entry));result.valid=result.valid&&valid;
  }catch(const MediaError& error){if(error.kind()!=ErrorKind::InvalidData)throw;Diagnostic warning{DiagnosticCode::CoherentChildUnreadable,{childName}};warning.causes.push_back(error.diagnostic());warnings.push_back(std::move(warning));}
 }
 stack.erase(number);
 std::stable_sort(result.entries.begin(),result.entries.end(),[](const auto&a,const auto&b){if((a.kind==EntryKind::Directory)!=(b.kind==EntryKind::Directory))return a.kind==EntryKind::Directory;const auto left=QString::fromUtf8(a.name.data(),qsizetype(a.name.size())),right=QString::fromUtf8(b.name.data(),qsizetype(b.name.size()));return QString::compare(left,right,Qt::CaseInsensitive)<0;});return result;
}
}
