#include "Track.h"
#include "Constants.h"
#include "Internal.h"
#include <QString>
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
namespace M=MediaFileSystems;
using namespace OpticalInternal;
bool OpticalMetadataLess::operator()(const std::string& a,const std::string& b)const{return QString::fromUtf8(a.data(),qsizetype(a.size())).compare(QString::fromUtf8(b.data(),qsizetype(b.size())),Qt::CaseInsensitive)<0;}
OpticalTrackIndex::OpticalTrackIndex(std::int32_t number,std::int64_t relative):number_(number),relative_(relative){if(number<0||number>OpticalConstants::MaximumIndex)throw M::MediaError(M::DiagnosticCode::OpticalIndexNumber);}
OpticalTrack::OpticalTrack(OpticalTrackOptions value){
    using namespace OpticalConstants;
    const auto positive=[](std::int64_t dimension,std::string_view field){if(dimension<=0)throw M::MediaError(M::DiagnosticCode::OpticalPositiveDimension,{std::string(field)});};
    const auto nonnegative=[](std::int64_t dimension,std::string_view field){if(dimension<0)throw M::MediaError(M::DiagnosticCode::OpticalNonNegativeDimension,{std::string(field)});};
    positive(value.sessionNumber,Session);positive(value.trackNumber,Track);nonnegative(value.firstSector,FirstSector);positive(value.sectorCount,SectorCount);positive(value.storedSectorSize,StoredSectorSize);nonnegative(value.userDataOffset,UserOffset);positive(value.userDataLength,UserLength);nonnegative(value.sourceOffset,SourceOffset);nonnegative(value.pregapSectors,Pregap);nonnegative(value.postgapSectors,Postgap);nonnegative(value.subchannelSourceOffset,SubchannelOffset);nonnegative(value.subchannelBytesPerSector,SubchannelBytes);nonnegative(value.storedPregapSectors,StoredPregap);nonnegative(value.subchannelStride,SubchannelStride);
    if(!value.dataSource)throw M::MediaError(M::DiagnosticCode::OpticalDataSource);
    if(value.userDataOffset>value.storedSectorSize-value.userDataLength)throw M::MediaError(M::DiagnosticCode::OpticalUserWindow);
    if(!fits(*value.dataSource,value.sourceOffset,multiply(value.sectorCount,value.storedSectorSize)))throw M::MediaError(M::DiagnosticCode::OpticalTrackSource);
    if(!value.subchannelSource&&(value.subchannelSourceOffset||value.subchannelBytesPerSector||value.subchannelStride))throw M::MediaError(M::DiagnosticCode::OpticalSubchannelSource);
    const auto stride=value.subchannelStride?value.subchannelStride:value.subchannelBytesPerSector;
    if(value.subchannelSource&&(value.subchannelBytesPerSector<=0||stride<value.subchannelBytesPerSector||!fits(*value.subchannelSource,value.subchannelSourceOffset,add(multiply(value.sectorCount-1,stride),value.subchannelBytesPerSector))))throw M::MediaError(M::DiagnosticCode::OpticalSubchannelRange);
    if(!value.storedPregapSectors&&value.storedPregapSourceOffset)throw M::MediaError(M::DiagnosticCode::OpticalPregapOffset);
    if(value.storedPregapSectors&&(!value.storedPregapSourceOffset||!fits(*value.dataSource,*value.storedPregapSourceOffset,multiply(value.storedPregapSectors,value.storedSectorSize))))throw M::MediaError(M::DiagnosticCode::OpticalPregapRange);
    value.subchannelStride=stride;std::stable_sort(value.indexes.begin(),value.indexes.end(),[](const auto& a,const auto& b){return a.number()<b.number();});state_=std::make_shared<const OpticalTrackOptions>(std::move(value));
}
std::future<void> OpticalTrack::readStoredSectorAsync(std::int64_t sector,M::MutableBytes destination,std::stop_token token)const{return std::async(std::launch::deferred,[track=state_,sector,destination,token]{
    if(destination.size()!=std::size_t(track->storedSectorSize))throw M::MediaError(M::DiagnosticCode::OpticalStoredDestination);
    std::int64_t offset;
    if(sector<0){if(sector< -track->storedPregapSectors||!track->storedPregapSourceOffset)throw M::MediaError(M::DiagnosticCode::OpticalStoredPregap);offset=add(*track->storedPregapSourceOffset,multiply(sector+track->storedPregapSectors,track->storedSectorSize));}
    else {relative(*track,sector);offset=add(track->sourceOffset,multiply(sector,track->storedSectorSize));}
    track->dataSource->readExactly(std::uint64_t(offset),destination,token);
});}
std::future<std::vector<std::uint8_t>> OpticalTrack::readStoredSectorAsync(std::int64_t sector,std::stop_token token)const{return std::async(std::launch::deferred,[track=state_,sector,token]{relative(*track,sector);return read(track->dataSource,add(track->sourceOffset,multiply(sector,track->storedSectorSize)),track->storedSectorSize,token);});}
std::future<std::vector<std::uint8_t>> OpticalTrack::readUserDataAsync(std::int64_t sector,std::stop_token token)const{return std::async(std::launch::deferred,[track=state_,sector,token]{relative(*track,sector);return read(track->dataSource,add(add(track->sourceOffset,multiply(sector,track->storedSectorSize)),track->userDataOffset),track->userDataLength,token);});}
std::future<std::optional<std::vector<std::uint8_t>>> OpticalTrack::readSubchannelAsync(std::int64_t sector,std::stop_token token)const{return std::async(std::launch::deferred,[track=state_,sector,token]()->std::optional<std::vector<std::uint8_t>>{relative(*track,sector);if(!track->subchannelSource)return {};return read(track->subchannelSource,add(track->subchannelSourceOffset,multiply(sector,track->subchannelStride)),track->subchannelBytesPerSector,token);});}
}
