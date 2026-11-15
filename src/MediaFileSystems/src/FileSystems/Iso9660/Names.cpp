#include "Internal.h"
#include <algorithm>
namespace RetroMediaLab::MediaFileSystems::Iso {
using namespace IsoConstants;
namespace {
QString trimEnd(QString text){while(!text.isEmpty()&&(text.back()==QChar(Null)||text.back()==QChar(Space)))text.chop(1);return text;}
std::string stripVersion(QString name){const auto position=name.indexOf(QChar(VersionSeparator));if(position>=0)name.truncate(position);if(name.endsWith(QChar(TrailingDot)))name.chop(1);return name.toUtf8().toStdString();}
std::size_t systemUse(Bytes record){const auto length=record[DirectoryFileIdentifierLengthOffset];return DirectoryFileIdentifierOffset+std::size_t(length)+((length&1)==0?1:0);}
bool reference(Bytes record){
    auto offset=systemUse(record);
    while(offset<=record.size()&&SuspHeaderLength<=record.size()-offset){
        const auto length=record[offset+SuspLengthOffset];if(length<SuspHeaderLength||length>record.size()-offset)return false;
        const auto item=record.subspan(offset,length);
        if(length>=SuspExtensionReferenceMinimumLength&&item[SuspVersionOffset]==SuspVersion&&item[0]==SuspExtensionReferenceSignatureFirstByte&&item[1]==SuspExtensionReferenceSignatureSecondByte){const auto count=item[SuspExtensionIdentifierLengthOffset];if(count<=length-SuspExtensionIdentifierOffset){const auto id=Optical::ascii(item.subspan(SuspExtensionIdentifierOffset,count));if(id==RockRidgeExtensionIdentifier||id==RockRidgeIeeeExtensionIdentifier)return true;}}
        offset+=length;
    }
    return false;
}
}
bool acceptDescriptor(Bytes data,Mode mode){
    if(mode!=Mode::Joliet)return data[0]==PrimaryVolumeDescriptorType;
    if(data[0]!=SupplementaryVolumeDescriptorType||data.size()<JolietDescriptorMinimum)return false;
    const auto escape=Optical::ascii(data.subspan(JolietEscapeOffset,JolietEscapeLength));return escape==JolietEscape1||escape==JolietEscape2||escape==JolietEscape3;
}
bool acceptFileSystem(const OpticalTrack& track,Bytes data,Mode mode,std::stop_token token){
    if(mode!=Mode::RockRidge)return true;
    const auto [block,length]=root(data);const auto bytes=extent(track,block,length,token);std::size_t offset=0;
    while(offset<bytes.size()){
        const auto count=bytes[offset];if(!count){offset=(offset/LogicalBlockSize+1)*LogicalBlockSize;continue;}
        if(count<DirectoryRecordMinimumLength||count>bytes.size()-offset)return false;
        if(reference(Bytes(bytes).subspan(offset,count)))return true;offset+=count;
    }
    return false;
}
std::string volumeName(Bytes bytes,Mode mode){return trimEnd(mode==Mode::Joliet?Optical::utf16be(bytes):QString::fromStdString(Optical::ascii(bytes))).toUtf8().toStdString();}
std::string fileName(Bytes bytes,Mode mode){if(mode==Mode::Joliet&&bytes.size()%UnicodeCodeUnitLength)throw MediaError(DiagnosticCode::JolietOddName);return stripVersion(mode==Mode::Joliet?Optical::utf16be(bytes):QString::fromStdString(Optical::ascii(bytes)));}
std::string entryName(Bytes record,std::string decoded,Mode mode){
    if(mode!=Mode::RockRidge)return decoded;
    auto offset=systemUse(record);if(offset>=record.size())return decoded;
    std::vector<std::uint8_t> name;bool found=false;
    while(offset<=record.size()&&SuspHeaderLength<=record.size()-offset){
        const auto length=record[offset+SuspLengthOffset];if(length<SuspHeaderLength||length>record.size()-offset)break;
        const auto item=record.subspan(offset,length);
        if(item[SuspVersionOffset]==SuspVersion&&item[0]==RockRidgeAlternateNameSignatureFirstByte&&item[1]==RockRidgeAlternateNameSignatureSecondByte){
            if(length<RockRidgeAlternateNameMinimumLength)break;
            const auto flags=item[RockRidgeAlternateNameFlagsOffset];if(flags&RockRidgeAlternateNameCurrentFlag)return std::string(Current);if(flags&RockRidgeAlternateNameParentFlag)return std::string(Parent);
            name.insert(name.end(),item.begin()+RockRidgeAlternateNameContentOffset,item.end());found=true;if(!(flags&RockRidgeAlternateNameContinueFlag))break;
        }
        offset+=length;
    }
    return found?Optical::utf8OrLatin1(name):decoded;
}
}
