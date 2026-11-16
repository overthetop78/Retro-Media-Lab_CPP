#include "Reader.h"
#include "../../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <QCryptographicHash>
#include <algorithm>
#include <unordered_map>
namespace RetroMediaLab::MediaEngine::WiiU {
namespace {
using namespace Constants;
std::string normalized(std::string_view extension){return !extension.empty()&&extension.front()!=ExtensionSeparator?std::string(1,ExtensionSeparator)+std::string(extension):std::string(extension);}
void writeWux(const BlockImage& blocks,const SeekableImageOutput& output,std::stop_token token){
    const auto logical=std::uint64_t(blocks.capacity()),count=logical/DefaultChunkBytes+(logical%DefaultChunkBytes!=0);if(!count||count>MaximumIndexCount)throw MediaError(DiagnosticCode::WiiUWrite);const auto indexEnd=HeaderBytes+count*IndexBytes,array=indexEnd+(DefaultChunkBytes-indexEnd%DefaultChunkBytes)%DefaultChunkBytes;
    std::vector<std::uint32_t> indexes;indexes.reserve(std::size_t(count));std::unordered_map<std::string,std::uint32_t> known;std::vector<std::uint8_t> buffer(DefaultChunkBytes,0),zeros(IndexBufferBytes,0);std::uint64_t padding=array;while(padding){checkCancellation(token);const auto size=std::size_t(std::min<std::uint64_t>(padding,zeros.size()));output.write(std::span(zeros).first(size));padding-=size;}
    std::uint32_t unique=0;for(std::uint64_t i=0;i<count;++i){checkCancellation(token);std::fill(buffer.begin(),buffer.end(),0);const auto offset=i*DefaultChunkBytes;const auto size=std::size_t(std::min<std::uint64_t>(DefaultChunkBytes,logical-offset));blocks.readExactly(std::int64_t(offset),std::span(buffer).first(size),token);const auto hash=QCryptographicHash::hash(QByteArrayView(reinterpret_cast<const char*>(buffer.data()),qsizetype(buffer.size())),QCryptographicHash::Sha256).toStdString();const auto [found,inserted]=known.emplace(hash,unique);if(inserted){output.write(buffer);++unique;}indexes.push_back(found->second);}
    output.seek(HeaderBytes);for(std::size_t offset=0;offset<indexes.size();){checkCancellation(token);const auto entries=std::min<std::size_t>(IndexBufferBytes/IndexBytes,indexes.size()-offset);for(std::size_t i=0;i<entries;++i)qToLittleEndian(indexes[offset+i],zeros.data()+i*IndexBytes);output.write(std::span(zeros).first(entries*IndexBytes));offset+=entries;}
    std::array<std::uint8_t,HeaderBytes> header{};qToLittleEndian(Magic0,header.data());qToLittleEndian(Magic1,header.data()+Magic1Offset);qToLittleEndian(std::uint32_t(DefaultChunkBytes),header.data()+ChunkOffset);qToLittleEndian(logical,header.data()+LengthOffset);output.seek(0);output.write(header);
}
}
const WriterCapabilities& Writer::capabilities()const{using namespace Constants;static const WriterCapabilities caps{std::string(WriterId),{std::string(FormatId)},{std::string(WudExtension),std::string(WuxExtension)},{RepresentationKind::Blocks},false};return caps;}
bool Writer::canWrite(const MediaDocument& document,std::string_view format,std::string_view extension)const{const auto* blocks=dynamic_cast<const BlockImage*>(&document.representation());return document.mediaKind()==MediaKind::Optical&&supportsTarget(format,normalized(extension))&&blocks&&wudLength(blocks->capacity())&&blocks->hasCompleteReadableCoverage();}
std::future<std::vector<std::filesystem::path>> Writer::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path path,std::string format,std::stop_token token)const{return std::async(std::launch::deferred,[document=std::move(document),path=std::move(path),format=std::move(format),token]{checkCancellation(token);if(!document)throw MediaError(DiagnosticCode::HardDiskRawNullDocument);const auto raw=path.extension().u8string();const std::string extension(raw.begin(),raw.end());if(!Writer{}.canWrite(*document,format,extension))throw MediaError(DiagnosticCode::WiiUWrite);const auto& blocks=dynamic_cast<const BlockImage&>(document->representation());if(sameIdentifier(extension,Constants::WuxExtension))writeAtomicSeekableImage(path,[&](const auto& output,auto stop){writeWux(blocks,output,stop);},token);else writeAtomicImage(path,[&](const auto& output,auto stop){std::vector<std::uint8_t> buffer(Constants::TransferBytes);std::int64_t offset=0;while(offset<blocks.capacity()){checkCancellation(stop);const auto count=std::size_t(std::min<std::int64_t>(buffer.size(),blocks.capacity()-offset));auto bytes=std::span(buffer).first(count);blocks.readExactly(offset,bytes,stop);output(bytes);offset+=count;}},token);return std::vector<std::filesystem::path>{path};});}
}
