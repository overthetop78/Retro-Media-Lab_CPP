#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::Cpm::Internal {
bool LogicalImage::available(std::int64_t offset,std::int64_t length)const{
    if(offset<0||length<0||offset>std::int64_t(bytes.size())-length)return false;if(length==0)return true;
    const auto first=offset/blockSize,last=(offset+length-1)/blockSize;for(auto block=first;block<=last;++block)if(missing.contains(std::int32_t(block))||truncated.contains(std::int32_t(block)))return false;return true;
}
LogicalImage flatten(const SectorImage& image,std::stop_token token){
    if(image.geometry().blockSize!=0&&image.blockCount()>Constants::MaximumBuffer/image.geometry().blockSize)throw MediaError(DiagnosticCode::ReadOutOfBounds,{},ErrorKind::Overflow);
    LogicalImage result;result.blockSize=image.geometry().blockSize;result.bytes.resize(std::size_t(image.blockCount()*result.blockSize));
    for(std::uint64_t number=0;number<image.blockCount();++number){checkCancellation(token);const auto block=image.block(number,token);if(!block){result.missing.insert(std::int32_t(number));continue;}const auto count=std::min(block->data.size(),std::size_t(result.blockSize));std::copy_n(block->data.begin(),count,result.bytes.begin()+std::size_t(number*result.blockSize));if(block->data.size()!=result.blockSize)result.truncated.insert(std::int32_t(number));}return result;
}
std::string decodePart(Bytes data){QByteArray clean;clean.reserve(qsizetype(data.size()));for(auto value:data)clean.push_back(char(value&Constants::AttributeMask));return QString::fromLatin1(clean).trimmed().toUtf8().toStdString();}
bool decodeName(Bytes entry,bool rejectLowercase,std::string& name){
    using namespace Constants;name.clear();if(entry.size()<DirectoryEntrySize)return false;
    for(auto index=FileNameOffset;index<ExtentLowOffset;++index){const auto value=entry[index]&AttributeMask;if(value!=Space&&(value<FirstPrintable||value>LastPrintable))return false;if(rejectLowercase&&value>=LowercaseFirst&&value<=LowercaseLast)return false;}
    auto stem=decodePart(entry.subspan(FileNameOffset,FileNameLength));auto extension=decodePart(entry.subspan(ExtensionOffset,ExtensionLength));if(stem.empty())return false;name=extension.empty()?std::move(stem):stem+Dot+extension;return true;
}
std::vector<std::int32_t> allocations(Bytes entry,bool wide){
    using namespace Constants;const auto count=wide?WideAllocationCount:NarrowAllocationCount;std::vector<std::int32_t> result;result.reserve(count);for(std::int32_t index=0;index<count;++index)result.push_back(wide?Binary::little<std::uint16_t>(entry,AllocationOffset+index*WideAllocationSize):entry[AllocationOffset+index]);return result;
}
bool plausibleLabel(std::string_view value){using namespace Constants;return !value.empty()&&std::all_of(value.begin(),value.end(),[](unsigned char c){return (c>=LowercaseFirst&&c<=LowercaseLast)||(c>=UppercaseFirst&&c<=UppercaseLast)||(c>=DigitFirst&&c<=DigitLast)||c==Space||c==Dash||c==Underscore||c==Dot;});}
std::int32_t score(const LogicalImage& image,const Layout& layout,bool rejectLowercase,std::stop_token token){
    std::int32_t result=0;for(std::int32_t index=0;index<layout.directoryEntries;++index){checkCancellation(token);const auto offset=std::int64_t(layout.directoryOffset)+std::int64_t(index)*Constants::DirectoryEntrySize;if(!image.available(offset,Constants::DirectoryEntrySize))continue;const auto entry=Bytes(image.bytes).subspan(std::size_t(offset),Constants::DirectoryEntrySize);std::string name;if(entry[0]<=Constants::MaximumUser&&decodeName(entry,rejectLowercase,name))++result;}return result;
}
bool looksLike(const LogicalImage& image,const Layout& layout,bool allowEmpty,bool rejectLowercase,std::stop_token token){
    using namespace Constants;std::int32_t active=0,unused=0;for(std::int32_t index=0;index<layout.directoryEntries;++index){checkCancellation(token);const auto offset=std::int64_t(layout.directoryOffset)+std::int64_t(index)*DirectoryEntrySize;if(!image.available(offset,DirectoryEntrySize))return false;const auto entry=Bytes(image.bytes).subspan(std::size_t(offset),DirectoryEntrySize);std::string name;if(entry[0]==UnusedEntry){++unused;continue;}if(entry[0]<=MaximumUser&&decodeName(entry,rejectLowercase,name))++active;else if(entry[0]!=VolumeLabelUser&&entry[0]!=PasswordLabelUser)return false;}return active>0||(allowEmpty&&unused==layout.directoryEntries);
}
std::optional<Layout> findDirectory(const LogicalImage& image,const Layout& model,std::int32_t step,bool allowEmpty,bool rejectLowercase,std::stop_token token){
    const auto length=std::int64_t(model.directoryEntries)*Constants::DirectoryEntrySize;for(std::int64_t offset=0;offset<=std::int64_t(image.bytes.size())-length;offset+=step){checkCancellation(token);auto candidate=model;candidate.directoryOffset=candidate.allocationOrigin=std::int32_t(offset);if(looksLike(image,candidate,allowEmpty,rejectLowercase,token))return candidate;}return {};
}
Directory directory(const LogicalImage& image,const Layout& layout,bool rejectLowercase,std::stop_token token){
    using namespace Constants;Directory result;for(std::int32_t index=0;index<layout.directoryEntries;++index){checkCancellation(token);const auto offset=std::int64_t(layout.directoryOffset)+std::int64_t(index)*DirectoryEntrySize;if(!image.available(offset,DirectoryEntrySize))continue;const auto entry=Bytes(image.bytes).subspan(std::size_t(offset),DirectoryEntrySize);const auto user=entry[0];if(user==UnusedEntry||user==PasswordLabelUser)continue;if(user==VolumeLabelUser){auto label=decodePart(entry.subspan(FileNameOffset,FileNameLength));if(plausibleLabel(label))result.volumeName=std::move(label);continue;}std::string name;if(user>MaximumUser||!decodeName(entry,rejectLowercase,name))continue;const auto number=entry[ExtentLowOffset]+(std::int32_t(entry[ExtentHighOffset])<<ExtentHighShift);result.extents.push_back({user,std::move(name),number,entry[RecordCountOffset],allocations(entry,layout.wide)});}return result;
}
std::vector<Group> groups(const std::vector<Extent>& extents){
    using Key=std::pair<std::uint8_t,std::string>;const auto compare=[](const Key& a,const Key& b){return a.first!=b.first?a.first<b.first:lessName(a.second,b.second);};std::map<Key,std::size_t,decltype(compare)> indexes(compare);std::vector<Group> result;
    for(const auto& extent:extents){const auto [position,inserted]=indexes.try_emplace({extent.user,extent.name},result.size());if(inserted)result.push_back({extent.user,extent.name,{extent}});else result[position->second].extents.push_back(extent);}return result;
}
}
