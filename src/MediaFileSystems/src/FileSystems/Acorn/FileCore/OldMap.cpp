#include <RetroMediaLab/MediaFileSystems/FileCore.h>
#include "Constants.h"
#include <algorithm>
#include <array>
namespace RetroMediaLab::MediaFileSystems::FileCore {
namespace {
using namespace Constants;
class OldMap final:public AddressResolver {
    std::uint64_t capacity_,free_=0;std::string name_;OldMapOptions options_;
public:
    OldMap(Bytes data,std::uint64_t capacity,OldMapOptions options):capacity_(capacity),options_(options){
        if(data.size()>=SecondNameOffset+HalfNameLength){std::array<std::uint8_t,NameLength> name{};for(int i=0;i<HalfNameLength;++i){name[i*PairCount]=data[FirstNameOffset+i];name[i*PairCount+1]=data[SecondNameOffset+i];}name_=decodeAdfsName(name);}
        if(data.size()>=FreeEntriesOffset+FreeEntryCount*UInt24Size){std::uint64_t sectors=0;for(int i=0;i<FreeEntryCount;++i){const auto offset=FreeEntriesOffset+i*UInt24Size;std::uint32_t value=0;for(int byte=0;byte<UInt24Size;++byte)value|=std::uint32_t(data[offset+byte])<<(byte*BitsPerByte);sectors+=value;}free_=std::min(capacity,sectors*FileCoreUnitSize);}
    }
    std::int32_t rootAddress() const noexcept override{return options_.rootAddress;}
    const std::string& volumeName() const noexcept override{return name_;}
    std::uint64_t freeBytes() const noexcept override{return free_;}
    std::optional<std::uint64_t> resolve(std::int32_t address,std::int64_t objectOffset,std::stop_token token) const override{
        checkCancellation(token);if(address<=0||objectOffset<0)return {};
        const auto base=std::uint64_t(address)*FileCoreUnitSize;
        if(base>=capacity_||std::uint64_t(objectOffset)>=capacity_-base)return {};
        auto position=base+std::uint64_t(objectOffset);
        if(options_.cylinders>0&&options_.heads>1&&options_.sectorsPerTrack>0){
            const auto sector=position/FileCoreUnitSize,side=std::uint64_t(options_.cylinders)*options_.sectorsPerTrack;
            const auto head=sector/side,cylinder=sector%side/options_.sectorsPerTrack;
            // Bound the remapping before multiplication, including inconsistent caller geometry.
            const auto maximumSector=(capacity_-1)/FileCoreUnitSize;
            if(cylinder>maximumSector/std::uint64_t(options_.heads))return {};
            const auto track=cylinder*std::uint64_t(options_.heads)+head;
            if(track>maximumSector/std::uint64_t(options_.sectorsPerTrack))return {};
            const auto physical=track*std::uint64_t(options_.sectorsPerTrack)+sector%options_.sectorsPerTrack;
            if(physical>maximumSector)return {};
            position=physical*FileCoreUnitSize+position%FileCoreUnitSize;
        }
        if(position>=capacity_)return {};return position;
    }
};
}
std::unique_ptr<AddressResolver> oldMap(Bytes data,std::uint64_t capacity){return oldMap(data,capacity,{Constants::OldRootAddress});}
std::unique_ptr<AddressResolver> oldMap(Bytes data,std::uint64_t capacity,OldMapOptions options){return std::make_unique<OldMap>(data,capacity,options);}
}
