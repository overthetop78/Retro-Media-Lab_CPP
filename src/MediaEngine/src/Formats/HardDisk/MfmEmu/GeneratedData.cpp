#include "Container.h"
#include "Signal.h"
#include "Constants.h"
#include "../Adaptec/Codec.h"
#include "../Adaptec/Constants.h"
#include "../Adaptec/Parameters.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <array>
#include <mutex>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::MfmEmu {
namespace {
void overlap(std::span<const std::uint8_t> source,std::uint64_t sourceOffset,MediaFileSystems::MutableBytes destination,std::uint64_t destinationOffset){const auto begin=std::max(sourceOffset,destinationOffset),end=std::min(sourceOffset+source.size(),destinationOffset+destination.size());if(end>begin)std::copy_n(source.begin()+std::size_t(begin-sourceOffset),std::size_t(end-begin),destination.begin()+std::size_t(begin-destinationOffset));}
class BlankData final:public MediaFileSystems::RandomAccessData {
    std::shared_ptr<const Header> header_;
public:
    explicit BlankData(std::shared_ptr<const Header> header):header_(std::move(header)){}
    std::uint64_t length()const noexcept override{return header_->endMarkerOffset()+Constants::MinimumTrackHeaderLength;}
    void readExactly(std::uint64_t offset,MediaFileSystems::MutableBytes destination,std::stop_token token)const override{
        using namespace Constants;checkCancellation(token);if(offset>length()||destination.size()>length()-offset)throw MediaError(DiagnosticCode::MfmReadRange);if(destination.empty())return;std::fill(destination.begin(),destination.end(),0);overlap(header_->bytes(),0,destination,offset);const auto start=std::max(offset,std::uint64_t(header_->headerLength())),end=std::min(offset+destination.size(),header_->endMarkerOffset());std::array<std::uint8_t,MinimumTrackHeaderLength> prefix{};qToLittleEndian(TrackMarker,prefix.data());
        if(end>start){const auto first=(start-header_->headerLength())/header_->trackRecordLength(),last=(end-1-header_->headerLength())/header_->trackRecordLength();for(auto track=first;track<=last;++track){checkCancellation(token);qToLittleEndian(std::int32_t(track/header_->heads()),prefix.data()+TrackCylinderOffset);qToLittleEndian(std::int32_t(track%header_->heads()),prefix.data()+TrackHeadOffset);overlap(prefix,header_->headerLength()+track*header_->trackRecordLength(),destination,offset);}}
        qToLittleEndian(EndAddress,prefix.data()+TrackCylinderOffset);qToLittleEndian(EndAddress,prefix.data()+TrackHeadOffset);overlap(prefix,header_->endMarkerOffset(),destination,offset);
    }
};
class FormattedData final:public MediaFileSystems::RandomAccessData {
    std::shared_ptr<const Header> header_;HardDiskGeometry geometry_;std::shared_ptr<const MediaFileSystems::RandomAccessData> blank_;std::vector<std::uint8_t> prefix_;mutable std::mutex mutex_;mutable std::optional<std::uint64_t> cachedTrack_;mutable std::vector<std::uint8_t> cachedBytes_;
    std::vector<std::uint8_t> createTrack(std::uint64_t track,std::stop_token token)const{
        namespace A=Adaptec::Constants;const auto count=geometry_.sectorsPerTrack(),size=geometry_.sectorSize();const auto sequential=(size==A::MinimumBlockSize&&count==A::SmallSequentialSectors)||(size==A::MediumBlockSize&&count==A::MediumSequentialSectors);std::vector<MediaFileSystems::SectorBlock> sectors(static_cast<std::size_t>(count));std::vector<bool> occupied(std::size_t(count),false);std::int32_t position{};
        for(std::int32_t logical=0;logical<count;++logical){checkCancellation(token);while(occupied[std::size_t(position)])if(++position==count)position=0;const auto address=track*count+logical,sourceOffset=address*size;auto& sector=sectors[std::size_t(position)];sector.logicalBlock=address;sector.address={std::int64_t(track/geometry_.heads()),std::int64_t(track%geometry_.heads()),std::int64_t(address)};sector.data.resize(std::size_t(size));if(sourceOffset<prefix_.size())std::copy_n(prefix_.begin()+std::size_t(sourceOffset),std::min(std::size_t(size),prefix_.size()-std::size_t(sourceOffset)),sector.data.begin());occupied[std::size_t(position)]=true;position=(position+(sequential?1:A::DefaultInterleave))%count;}
        std::array<std::uint8_t,A::ParameterFlagCount> flags{};if(track==0)Adaptec::writeInitialParameters(flags,size,std::int32_t(geometry_.cylinders()),geometry_.heads());const auto flux=Adaptec::encode(sectors,sequential,A::NominalIndexNanoseconds,0,track==0?std::span<const std::uint8_t>(flags):std::span<const std::uint8_t>{},token);std::vector<std::uint32_t> transitions;std::uint32_t time{};for(auto interval:flux.intervals){time+=interval;transitions.push_back(time);}return writeSignalWindow(std::vector<std::uint8_t>(std::size_t(header_->trackDataLength())),header_->sampleRate(),header_->startNanoseconds(),A::NominalIndexNanoseconds,0,A::NominalIndexNanoseconds,transitions,false,token);
    }
public:
    FormattedData(std::shared_ptr<const Header> header,HardDiskGeometry geometry,std::vector<std::uint8_t> prefix):header_(std::move(header)),geometry_(std::move(geometry)),blank_(std::make_shared<const BlankData>(header_)),prefix_(std::move(prefix)){if(prefix_.size()>std::uint64_t(geometry_.capacity()))throw MediaError(DiagnosticCode::MfmCreation);}
    std::uint64_t length()const noexcept override{return blank_->length();}
    void readExactly(std::uint64_t offset,MediaFileSystems::MutableBytes destination,std::stop_token token)const override{
        blank_->readExactly(offset,destination,token);std::lock_guard lock(mutex_);const auto first=std::max(offset,std::uint64_t(header_->headerLength())),end=std::min(offset+destination.size(),header_->endMarkerOffset());if(end<=first)return;const auto firstTrack=(first-header_->headerLength())/header_->trackRecordLength(),lastTrack=(end-1-header_->headerLength())/header_->trackRecordLength();
        for(auto track=firstTrack;track<=lastTrack;++track){checkCancellation(token);const auto dataStart=header_->headerLength()+track*header_->trackRecordLength()+header_->trackHeaderLength();const auto from=std::max(offset,dataStart),until=std::min(offset+destination.size(),dataStart+header_->trackDataLength());if(until<=from)continue;if(!cachedTrack_||*cachedTrack_!=track){auto bytes=createTrack(track,token);cachedBytes_=std::move(bytes);cachedTrack_=track;}std::copy_n(cachedBytes_.begin()+std::size_t(from-dataStart),std::size_t(until-from),destination.begin()+std::size_t(from-offset));}
    }
};
}
std::shared_ptr<const MediaFileSystems::RandomAccessData> blankData(std::shared_ptr<const Header> header){return std::make_shared<const BlankData>(std::move(header));}
std::shared_ptr<const MediaFileSystems::RandomAccessData> formattedData(std::shared_ptr<const Header> header,HardDiskGeometry geometry,std::vector<std::uint8_t> prefix){return std::make_shared<const FormattedData>(std::move(header),std::move(geometry),std::move(prefix));}
}
