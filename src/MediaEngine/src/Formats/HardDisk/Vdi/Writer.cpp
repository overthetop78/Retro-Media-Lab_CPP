#include "Container.h"
#include "../../../Sources/AtomicImageWriter.h"
#include "../Variant.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <QUuid>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Vdi {
namespace {
std::vector<std::uint8_t> createHeader(const BlockImage& blocks,bool fixed,std::uint32_t dataOffset,std::uint32_t count,std::uint32_t allocated){
    using namespace Constants;std::vector<std::uint8_t> header(HeaderSize);
    const auto put=[&](std::size_t offset,std::uint32_t value){qToLittleEndian(value,header.data()+offset);};
    std::copy(Banner.begin(),Banner.end(),header.begin());std::copy(Description.begin(),Description.end(),header.begin()+DescriptionOffset);
    put(SignatureOffset,Signature);put(VersionOffset,Version);put(HeaderSizeOffset,DeclaredHeaderSize);put(ImageTypeOffset,fixed?FixedType:DynamicType);put(MapOffset,HeaderSize);put(DataOffset,dataOffset);
    if(blocks.geometry()){const auto& geometry=*blocks.geometry();if(std::uint64_t(geometry.cylinders())>MaximumUnsignedField)throw MediaError(DiagnosticCode::VdiGeometry);put(CylindersOffset,std::uint32_t(geometry.cylinders()));put(HeadsOffset,std::uint32_t(geometry.heads()));put(SectorsOffset,std::uint32_t(geometry.sectorsPerTrack()));}
    put(SectorSizeOffset,SectorSize);qToLittleEndian(std::uint64_t(blocks.capacity()),header.data()+DiskSizeOffset);put(BlockSizeOffset,DefaultBlockSize);put(BlocksCountOffset,count);put(BlocksAllocatedOffset,allocated);
    for(auto offset:{ImageUuidOffset,ModifyUuidOffset}){auto bytes=QUuid::createUuid().toRfc4122();qToLittleEndian(qFromBigEndian<quint32>(bytes.constData()),bytes.data());qToLittleEndian(qFromBigEndian<quint16>(bytes.constData()+UuidMiddleOffset),bytes.data()+UuidMiddleOffset);qToLittleEndian(qFromBigEndian<quint16>(bytes.constData()+UuidLastOffset),bytes.data()+UuidLastOffset);std::copy(bytes.begin(),bytes.end(),header.begin()+offset);}
    return header;
}
void writeContainer(const BlockImage& blocks,bool fixed,const SeekableImageOutput& output,std::stop_token token){
    using namespace Constants;const auto count=(std::uint64_t(blocks.capacity())+DefaultBlockSize-1)/DefaultBlockSize;
    const auto mapSize=(count*EntryBytes+SectorSize-1)/SectorSize*SectorSize,dataOffset=HeaderSize+mapSize;
    if(count>MaximumSignedField||dataOffset>MaximumSignedField)throw MediaError(DiagnosticCode::VdiTableSize);
    std::vector<std::uint32_t> map(std::size_t(count),Unallocated);std::vector<std::uint8_t> buffer(DefaultBlockSize);
    for(std::uint64_t written=0;written<dataOffset;){const auto length=std::min(std::uint64_t(buffer.size()),dataOffset-written);output.write(std::span(buffer).first(std::size_t(length)));written+=length;}
    std::uint32_t allocated{};
    for(std::uint64_t logical=0;logical<count;++logical){checkCancellation(token);std::fill(buffer.begin(),buffer.end(),0);const auto address=std::int64_t(logical*DefaultBlockSize);const auto length=std::min(std::int64_t(DefaultBlockSize),blocks.capacity()-address);blocks.readExactly(address,std::span(buffer).first(std::size_t(length)),token);
        if(!fixed&&std::all_of(buffer.begin(),buffer.begin()+std::size_t(length),[](auto byte){return byte==0;})){map[std::size_t(logical)]=Discarded;continue;}
        map[std::size_t(logical)]=allocated++;output.write(buffer);
    }
    const auto end=output.position();output.seek(0);output.write(createHeader(blocks,fixed,std::uint32_t(dataOffset),std::uint32_t(count),allocated));
    std::vector<std::uint8_t> bytes(std::size_t(mapSize),0);for(std::size_t i=0;i<map.size();++i)qToLittleEndian(map[i],bytes.data()+i*EntryBytes);output.write(bytes);output.seek(end);
}
}
const WriterCapabilities& Writer::capabilities()const {using namespace Constants;static const WriterCapabilities caps{std::string(WriterId),{std::string(FormatId)},{std::string(Extension)},{RepresentationKind::Blocks},false};return caps;}
bool Writer::canWrite(const MediaDocument& document,std::string_view format,std::string_view extension)const{
    const auto* blocks=dynamic_cast<const BlockImage*>(&document.representation());return supportsTarget(format,extension)&&document.mediaKind()==MediaKind::HardDisk&&blocks&&blocks->capacity()>0&&blocks->logicalBlockSize()==Constants::SectorSize&&blocks->hasCompleteReadableCoverage();
}
std::future<std::vector<std::filesystem::path>> Writer::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path path,std::string target,std::stop_token token)const {
    return std::async(std::launch::deferred,[document=std::move(document),path=std::move(path),target=std::move(target),token]{
        if(!document)throw MediaError(DiagnosticCode::HardDiskRawNullDocument);const auto ext=path.extension().u8string();if(!Writer{}.canWrite(*document,target,std::string(ext.begin(),ext.end())))throw MediaError(DiagnosticCode::VdiWrite);
        const auto& blocks=dynamic_cast<const BlockImage&>(document->representation());const auto fixed=HardDiskContainers::selectFixedVariant(*document,blocks,Constants::FormatId);
        writeAtomicSeekableImage(path,[&](const auto& output,auto stop){writeContainer(blocks,fixed,output,stop);},token);return std::vector<std::filesystem::path>{path};
    });
}
}
