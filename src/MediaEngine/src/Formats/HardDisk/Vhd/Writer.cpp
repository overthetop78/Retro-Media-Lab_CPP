#include "Writing.h"
#include "../Variant.h"
#include "../../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Vhd {
namespace {
void writeDynamic(const BlockImage& blocks,const SeekableImageOutput& output,std::stop_token token){
    using namespace Constants;const auto count=(std::uint64_t(blocks.capacity())+DefaultBlockSize-1)/DefaultBlockSize;const auto padded=(count*EntryBytes+SectorSize-1)/SectorSize*SectorSize;
    if(count>MaximumUnsignedField||padded>MaximumSignedField)throw MediaError(DiagnosticCode::VhdWriteTable);
    const auto footer=createFooter(blocks,DynamicType,FooterSize);output.write(footer);output.write(createDynamicHeader(std::uint32_t(count)));const auto tableOffset=output.position();
    std::vector<std::uint8_t> tableBytes(std::size_t(padded),0);output.write(tableBytes);std::vector<std::uint32_t> table(std::size_t(count),UnusedEntry);
    std::vector<std::uint8_t> data(DefaultBlockSize);const auto bitmapBytes=(DefaultBlockSize/SectorSize+ByteBits-1)/ByteBits,bitmapStorage=(bitmapBytes+SectorSize-1)/SectorSize*SectorSize;std::vector<std::uint8_t> bitmap(bitmapStorage);
    for(std::uint64_t i=0;i<count;++i){checkCancellation(token);std::fill(data.begin(),data.end(),0);const auto address=std::int64_t(i*DefaultBlockSize),length=std::min(std::int64_t(DefaultBlockSize),blocks.capacity()-address);blocks.readExactly(address,std::span(data).first(std::size_t(length)),token);
        if(std::all_of(data.begin(),data.begin()+std::size_t(length),[](auto byte){return byte==0;}))continue;
        const auto position=output.position();if(position%SectorSize)throw MediaError(DiagnosticCode::VhdWriteAlignment);const auto sectorOffset=position/SectorSize;if(sectorOffset>MaximumUnsignedField)throw MediaError(DiagnosticCode::VhdWriteOffset);table[std::size_t(i)]=std::uint32_t(sectorOffset);
        std::fill(bitmap.begin(),bitmap.end(),0);const auto sectors=(std::uint64_t(length)+SectorSize-1)/SectorSize;
        for(std::uint64_t sector=0;sector<sectors;++sector)bitmap[std::size_t(sector/ByteBits)]|=std::uint8_t(BitmapFirstBit>>(sector%ByteBits));output.write(bitmap);output.write(data);
    }
    output.write(footer);const auto end=output.position();output.seek(tableOffset);for(std::size_t i=0;i<table.size();++i)qToBigEndian(table[i],tableBytes.data()+i*EntryBytes);output.write(tableBytes);output.seek(end);
}
}
const WriterCapabilities& Writer::capabilities()const{using namespace Constants;static const WriterCapabilities caps{std::string(WriterId),{std::string(FormatId)},{std::string(Extension)},{RepresentationKind::Blocks},false};return caps;}
bool Writer::canWrite(const MediaDocument& document,std::string_view target,std::string_view extension)const{const auto* blocks=dynamic_cast<const BlockImage*>(&document.representation());return supportsTarget(target,extension)&&document.mediaKind()==MediaKind::HardDisk&&blocks&&blocks->capacity()>0&&blocks->logicalBlockSize()==Constants::SectorSize&&blocks->hasCompleteReadableCoverage();}
std::future<std::vector<std::filesystem::path>> Writer::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path path,std::string target,std::stop_token token)const{
    return std::async(std::launch::deferred,[document=std::move(document),path=std::move(path),target=std::move(target),token]{using namespace Constants;if(!document)throw MediaError(DiagnosticCode::HardDiskRawNullDocument);const auto extension=path.extension().u8string();if(!Writer{}.canWrite(*document,target,std::string(extension.begin(),extension.end())))throw MediaError(DiagnosticCode::VhdWrite);
        const auto& blocks=dynamic_cast<const BlockImage&>(document->representation());
        if(HardDiskContainers::selectFixedVariant(*document,blocks,FormatId))writeAtomicImage(path,[&](const auto& output,auto stop){copyBlocks(blocks,output,stop);output(createFooter(blocks,FixedType,NoDataOffset));},token);
        else writeAtomicSeekableImage(path,[&](const auto& output,auto stop){writeDynamic(blocks,output,stop);},token);
        return std::vector<std::filesystem::path>{path};
    });
}
}
