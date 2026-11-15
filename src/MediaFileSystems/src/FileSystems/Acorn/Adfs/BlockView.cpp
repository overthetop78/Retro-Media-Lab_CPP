#include "Internal.h"
#include <limits>
namespace RetroMediaLab::MediaFileSystems::Adfs {
namespace {
class BlockView final:public SectorImage {
    const BlockMedia& source_;std::uint64_t start_,length_;SectorGeometry geometry_;
public:
    BlockView(const BlockMedia& source,std::uint64_t start,std::uint64_t length,std::string_view id):source_(source),start_(start),length_(length){geometry_.blockSize=Constants::UnitSize;geometry_.formatId=id;geometry_.addressing=SectorAddressingKind::Logical;}
    const SectorGeometry& geometry()const noexcept override{return geometry_;}
    std::uint64_t blockCount()const noexcept override{return length_/Constants::UnitSize;}
    std::uint64_t capacity()const noexcept override{return length_;}
    bool allowsVariableBlockSize()const noexcept override{return false;}
    bool containsBlock(std::uint64_t index)const noexcept override{return index<blockCount();}
    std::optional<SectorAddress> blockAddress(std::uint64_t index)const override{if(!containsBlock(index))return {};return SectorAddress{0,0,std::int64_t(index)};}
    std::shared_ptr<const SectorBlock> block(std::uint64_t index,std::stop_token token)const override{
        checkCancellation(token);if(!containsBlock(index))return {};
        auto result=std::make_shared<SectorBlock>();result->logicalBlock=index;result->address.number=std::int64_t(index);result->integrityValid=std::nullopt;result->data.resize(Constants::UnitSize);
        try{source_.readExactly(start_+index*Constants::UnitSize,result->data,token);}
        catch(const MediaError& error){if(error.kind()==ErrorKind::Io||error.kind()==ErrorKind::InvalidData)return {};throw;}
        return result;
    }
    // AvailableBlocks is empty in the C# borrowed block-volume view; direct indexed reads remain available.
    void visitAvailableBlocks(const std::function<bool(std::uint64_t)>&,std::stop_token token)const override{checkCancellation(token);}
};
bool validVolume(const BlockMedia& source,std::uint64_t start,std::uint64_t length){
    return length>=Constants::FirstFreeSector*Constants::UnitSize&&length%Constants::UnitSize==0&&start<=source.capacity()&&length<=source.capacity()-start&&length/Constants::UnitSize<=std::uint64_t(std::numeric_limits<std::int32_t>::max());
}
}
bool canRead(const BlockMedia& source,std::uint64_t start,std::uint64_t length,std::string_view id,std::stop_token token){checkCancellation(token);return validVolume(source,start,length)&&canRead(BlockView(source,start,length,id),token);}
FileSystemVolume read(const BlockMedia& source,std::uint64_t start,std::uint64_t length,std::string_view id,std::stop_token token){checkCancellation(token);if(!validVolume(source,start,length))throw MediaError(DiagnosticCode::AdfsBlockVolume);return read(BlockView(source,start,length,id),token);}
}
