#include <RetroMediaLab/MediaFileSystems/AppleRecognition.h>
#include <RetroMediaLab/MediaFileSystems/AppleRecognitionConstants.h>
#include "../../Primitives/Binary.h"
#include <QString>
namespace RetroMediaLab::MediaFileSystems::Apple {
using namespace RecognitionConstants;
std::optional<std::uint16_t> macintoshSignature(Bytes data) noexcept {
    if(data.size()<MacSignatureOffset+SignatureBytes)return {};
    return Binary::big<std::uint16_t>(data,MacSignatureOffset);
}
bool supportedMacintoshSignature(std::uint16_t value) noexcept{return value==MfsSignature||value==HfsSignature;}
std::optional<MacintoshKind> identifyMacintosh(Bytes data) noexcept{
    const auto value=macintoshSignature(data);
    if(value==MfsSignature)return MacintoshKind::Mfs;
    if(value==HfsSignature)return MacintoshKind::Hfs;
    return {};
}
bool validDosVtoc(Bytes data,std::int32_t tracks,std::int32_t sectors) noexcept {
    if(data.size()<DosSectorBytes+sizeof(std::uint16_t))return false;
    return data[DosCatalogTrack]>0&&data[DosCatalogTrack]<tracks&&data[DosCatalogSector]<sectors
        &&data[DosTrackSectors]==sectors&&Binary::little<std::uint16_t>(data,DosSectorBytes)==DosSectorSize;
}
bool validProDosHeader(Bytes root) noexcept{
    if(root.size()!=BlockSize)return false;
    const auto storage=root[ProDosHeader]>>ProDosStorageShift;
    const auto length=root[ProDosHeader]&ProDosNameMask;
    return storage==ProDosVolumeHeader&&length>0&&length<=ProDosNameMask&&root[ProDosEntryLength]==ProDosEntrySize;
}
bool knownLisaVersion(std::uint16_t version) noexcept{return std::find(LisaVersions.begin(),LisaVersions.end(),version)!=LisaVersions.end();}
bool printableLisaName(Bytes name) noexcept{return std::all_of(name.begin(),name.end(),[](auto c){return c>=PrintableFirst&&c<=PrintableLast;});}
bool validLisaHeader(Bytes page) noexcept{
    if(page.size()<LisaMinimumLength)return false;
    const auto length=page[LisaNameLength];
    return knownLisaVersion(Binary::big<std::uint16_t>(page,LisaVersionOffset))&&length>0&&length<=LisaMaximumName
        &&LisaNameOffset+length<=page.size()&&printableLisaName(page.subspan(LisaNameOffset,length));
}
std::string decodeLisaName(Bytes bytes){
    const auto end=std::find(bytes.begin(),bytes.end(),std::uint8_t(Null));
    bytes=bytes.first(std::size_t(end-bytes.begin()));
    while(!bytes.empty()&&bytes.front()==Space)bytes=bytes.subspan(1);
    while(!bytes.empty()&&bytes.back()==Space)bytes=bytes.first(bytes.size()-1);
    return QString::fromLatin1(reinterpret_cast<const char*>(bytes.data()),qsizetype(bytes.size())).toUtf8().toStdString();
}
bool containsSosMarker(Bytes data) noexcept{
    const auto size=std::min(SosSearchLength,data.size());
    if(size<SosMarker.size())return false;
    for(std::size_t offset=0;offset<=size-SosMarker.size();++offset){
        bool matches=true;
        for(std::size_t index=0;index<SosMarker.size();++index){
            auto c=data[offset+index];if(c>=LowerFirst&&c<=LowerLast)c-=CaseDelta;
            if(c!=std::uint8_t(SosMarker[index])){matches=false;break;}
        }
        if(matches)return true;
    }
    return false;
}
}
