#include "AdfsDocument.h"
#include "CatalogConstants.h"
#include <RetroMediaLab/MediaFileSystems/Adfs.h>
namespace RetroMediaLab::MediaFileSystems::Composition {
namespace {
// Borrowed only during the synchronous probe/read; ADFS materializes its result.
class BorrowedBlocks final:public RandomAccessData {
    const BlockRepresentation& blocks_;
public:
    explicit BorrowedBlocks(const BlockRepresentation& blocks):blocks_(blocks){}
    std::uint64_t length()const noexcept override{return std::uint64_t(blocks_.capacity());}
    void readExactly(std::uint64_t offset,MutableBytes output,std::stop_token token)const override {
        checkCancellation(token);
        if(offset>length()||output.size()>length()-offset)throw MediaError(DiagnosticCode::ReadOutOfBounds);
        blocks_.readExactlyAsync(std::int64_t(offset),output,token).get();
    }
};
std::optional<BlockMedia> blocks(const ImageDocument& document){
    const auto* source=dynamic_cast<const BlockRepresentation*>(&document.representation());
    if(!source||source->capacity()<0)return {};
    return BlockMedia(std::make_shared<BorrowedBlocks>(*source));
}
class AdfsDocumentReader final:public SectorReader {
public:
    std::string_view id()const noexcept override{return CatalogConstants::Reader2;}
    std::span<const std::string_view> catalogFormatIds()const noexcept override{return CatalogConstants::Formats2;}
    bool canRead(const SectorImage& image,std::stop_token token)const override{return Adfs::canRead(image,token);}
    FileSystemVolume read(const SectorImage& image,std::stop_token token)const override{return Adfs::read(image,token);}
    bool canRead(const ImageDocument& document,const VolumeDescriptor& volume,std::stop_token token)const override {
        checkCancellation(token);
        if(dynamic_cast<const SectorRepresentation*>(&document.representation()))return SectorReader::canRead(document,volume,token);
        const auto source=blocks(document);
        return source&&Adfs::canRead(*source,volume.start(),volume.length(),document.formatId(),token);
    }
    FileSystemVolume read(const ImageDocument& document,const VolumeDescriptor& volume,std::stop_token token)const override {
        checkCancellation(token);
        if(dynamic_cast<const SectorRepresentation*>(&document.representation()))return SectorReader::read(document,volume,token);
        const auto source=blocks(document);if(!source)throw MediaError(DiagnosticCode::AdfsBlockVolume);
        return Adfs::read(*source,volume.start(),volume.length(),document.formatId(),token);
    }
};
}
std::shared_ptr<const SectorReader> createAdfsDocumentReader(){return std::make_shared<AdfsDocumentReader>();}
}
