#include "Container.h"
#include "../../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <QCryptographicHash>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::ChdHardDisk {
namespace {
void writeContainer(const BlockImage& blocks,const SeekableImageOutput& output,std::stop_token token){
    using namespace Constants;const auto count=std::uint64_t(blocks.capacity())/DefaultHunkBytes+(blocks.capacity()%DefaultHunkBytes!=0);
    if(count>MaximumBuffer/UncompressedMapBytes)throw MediaError(DiagnosticCode::ChdHardDiskWriteLayout);const auto mapLength=count*UncompressedMapBytes,dataOffset=(HeaderBytes+mapLength+DefaultHunkBytes-1)/DefaultHunkBytes*DefaultHunkBytes;
    const auto lastOffset=dataOffset+(count-1)*DefaultHunkBytes;if(lastOffset/DefaultHunkBytes>MaximumField)throw MediaError(DiagnosticCode::ChdHardDiskWriteLayout);
    std::vector<std::uint8_t> map(std::size_t(mapLength),0),hunk(DefaultHunkBytes,0);QCryptographicHash hash(QCryptographicHash::Sha1);
    for(std::uint64_t index=0;index<count;++index){checkCancellation(token);std::fill(hunk.begin(),hunk.end(),0);const auto address=std::int64_t(index*DefaultHunkBytes);const auto length=std::size_t(std::min(std::int64_t(DefaultHunkBytes),blocks.capacity()-address));blocks.readExactly(address,std::span(hunk).first(length),token);
        hash.addData(QByteArrayView(reinterpret_cast<const char*>(hunk.data()),qsizetype(length)));const auto offset=dataOffset+index*DefaultHunkBytes;qToBigEndian(std::uint32_t(offset/DefaultHunkBytes),map.data()+std::size_t(index)*UncompressedMapBytes);output.seek(offset);output.write(hunk);
    }
    const auto& geometry=*blocks.geometry();const std::array<std::int64_t,GeometryFields> values{geometry.cylinders(),geometry.heads(),geometry.sectorsPerTrack(),geometry.sectorSize()};std::string metadata;
    for(int i=0;i<GeometryFields;++i){if(i)metadata+=GeometrySeparator;metadata+=GeometryPrefixes[i];metadata+=std::to_string(values[i]);}
    if(metadata.size()>MetadataLengthMask)throw MediaError(DiagnosticCode::ChdHardDiskWriteLayout);const auto metadataOffset=dataOffset+count*DefaultHunkBytes;
    std::array<std::uint8_t,MetadataHeaderBytes> record{};qToBigEndian(MetadataTag,record.data());qToBigEndian(std::uint32_t(metadata.size()),record.data()+MetadataLengthOffset);output.seek(metadataOffset);output.write(record);output.write(std::span(reinterpret_cast<const std::uint8_t*>(metadata.data()),metadata.size()));
    output.seek(HeaderBytes);output.write(map);std::array<std::uint8_t,HeaderBytes> header{};std::copy(Signature.begin(),Signature.end(),header.begin());qToBigEndian(std::uint32_t(HeaderBytes),header.data()+HeaderSizeOffset);qToBigEndian(std::uint32_t(Version5),header.data()+VersionOffset);
    qToBigEndian(std::uint64_t(blocks.capacity()),header.data()+LogicalOffset);qToBigEndian(std::uint64_t(HeaderBytes),header.data()+MapOffset);qToBigEndian(metadataOffset,header.data()+MetadataOffset);qToBigEndian(DefaultHunkBytes,header.data()+HunkOffset);qToBigEndian(std::uint32_t(blocks.logicalBlockSize()),header.data()+UnitOffset);const auto digest=hash.result();std::copy_n(digest.begin(),Sha1Bytes,header.begin()+Sha1Offset);output.seek(0);output.write(header);
}
}
const WriterCapabilities& Writer::capabilities()const{using namespace Constants;static const WriterCapabilities caps{std::string(WriterId),{std::string(FormatId)},{std::string(Extension)},{RepresentationKind::Blocks},false};return caps;}
bool Writer::canWrite(const MediaDocument& document,std::string_view format,std::string_view extension)const{
    const auto* blocks=dynamic_cast<const BlockImage*>(&document.representation());return supportsTarget(format,extension)&&document.mediaKind()==MediaKind::HardDisk&&blocks&&blocks->capacity()>0&&blocks->geometry()&&blocks->geometry()->capacity()==blocks->capacity()&&blocks->geometry()->sectorSize()==blocks->logicalBlockSize()&&blocks->logicalBlockSize()>0&&Constants::DefaultHunkBytes%blocks->logicalBlockSize()==0&&blocks->hasCompleteReadableCoverage();
}
std::future<std::vector<std::filesystem::path>> Writer::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path path,std::string format,std::stop_token token)const{
    return std::async(std::launch::deferred,[document=std::move(document),path=std::move(path),format=std::move(format),token]{if(!document)throw MediaError(DiagnosticCode::HardDiskRawNullDocument);const auto extension=path.extension().u8string();if(!Writer{}.canWrite(*document,format,std::string(extension.begin(),extension.end())))throw MediaError(DiagnosticCode::ChdHardDiskWrite);
        const auto& blocks=dynamic_cast<const BlockImage&>(document->representation());writeAtomicSeekableImage(path,[&](const auto& output,auto stop){writeContainer(blocks,output,stop);},token);return std::vector<std::filesystem::path>{path};});
}
}
