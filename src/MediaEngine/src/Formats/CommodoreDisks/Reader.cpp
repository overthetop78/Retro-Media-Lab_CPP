#include "Images.h"
#include <RetroMediaLab/MediaFileSystems/CommodoreDos.h>
#include "Constants.h"
#include "../../Models/Sectors/SectorImages.h"
#include <RetroMediaLab/MediaFileSystems/CommodoreGeometry.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
namespace RetroMediaLab::MediaEngine::CommodoreDisks {
namespace {
using namespace MediaFileSystems;
namespace G=MediaFileSystems::Commodore;
bool same(std::string_view a,std::string_view b) {
    if(a.size()!=b.size())return false;
    for(std::size_t n=0;n<a.size();++n){const auto c=a[n];if((c>=Constants::UpperFirst&&c<=Constants::UpperLast?char(c+Constants::CaseDelta):c)!=b[n])return false;}return true;
}
std::string_view display(Kind kind) {switch(kind){case Kind::D64:return Constants::D64;case Kind::D71:return Constants::D71;case Kind::D81:return Constants::D81;}return {};}
class ZonedImage final:public SectorImage {
    std::shared_ptr<const RandomAccessData> source_;
    SectorGeometry geometry_;
    Layout layout_;
    std::vector<std::uint8_t> errors_;
public:
    ZonedImage(Kind kind,std::shared_ptr<const RandomAccessData> source,Layout layout,std::stop_token token):source_(std::move(source)),geometry_{std::string(formatId(kind)),G::Constants::SectorSize,layout.tracksPerSide,layout.sides,G::Constants::MaximumSectors},layout_(layout) {
        if(layout.errorMapOffset){errors_.resize(layout.dataBlockCount);source_->readExactly(*layout.errorMapOffset,errors_,token);}
    }
    const SectorGeometry& geometry()const noexcept override{return geometry_;}
    std::uint64_t blockCount()const noexcept override{return layout_.dataBlockCount;}
    std::uint64_t capacity()const noexcept override{return blockCount()*geometry_.blockSize;}
    bool allowsVariableBlockSize()const noexcept override{return false;}
    bool containsBlock(std::uint64_t n)const noexcept override{return n<blockCount();}
    std::optional<SectorAddress> blockAddress(std::uint64_t n)const override {
        if(!containsBlock(n))return {};
        const auto address=G::address1541(std::int32_t(n),std::int32_t(layout_.tracksPerSide),std::int32_t(layout_.sides));
        return SectorAddress{G::cylinder1541(address.track),address.side,address.sector};
    }
    std::shared_ptr<const SectorBlock> block(std::uint64_t n,std::stop_token token)const override {
        MediaEngine::checkCancellation(token);if(!containsBlock(n))return nullptr;
        auto result=std::make_shared<SectorBlock>();result->logicalBlock=n;result->address=*blockAddress(n);result->data.resize(geometry_.blockSize);
        if(layout_.errorMapOffset)result->diagnosticCode=errors_[std::size_t(n)];
        result->integrityValid=!result->diagnosticCode||*result->diagnosticCode==Constants::NoError;
        source_->readExactly(n*geometry_.blockSize,result->data,token);return result;
    }
    void visitAvailableBlocks(const std::function<bool(std::uint64_t)>& visit,std::stop_token token)const override {
        if(!visit)return;
        for(std::uint64_t n=0;n<blockCount();++n){MediaEngine::checkCancellation(token);if(!visit(n))break;}
    }
};
}
bool canRead(Kind kind,const MediaFileSystems::RandomAccessData& source,std::optional<std::string_view> requested,std::stop_token token) {
    checkCancellation(token);return (!requested||same(*requested,formatId(kind)))&&findLayout(kind,source.length());
}
std::shared_ptr<const MediaFileSystems::SectorImage> read(Kind kind,std::shared_ptr<const MediaFileSystems::RandomAccessData> source,std::stop_token token) {
    checkCancellation(token);
    if(!source)throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::InvalidSource);
    const auto layout=findLayout(kind,source->length());
    if(!layout){std::string accepted;for(const auto& candidate:layouts(kind)){if(!accepted.empty())accepted+=Constants::SizeSeparator;accepted+=std::to_string(candidate.imageLength);}throw MediaEngine::MediaError(MediaEngine::DiagnosticCode::CommodoreImageLength,{std::string(display(kind)),std::to_string(source->length()),accepted});}
    if(kind==Kind::D81) {
        MediaFileSystems::SectorGeometry geometry{std::string(formatId(kind)),G::Constants::SectorSize,G::Constants::Cylinders1581,G::Constants::Heads1581,G::Constants::BlocksPerTrack1581};
        SourceSectorMetadata metadata;metadata.firstSectorNumber=0;metadata.trustCompletePayload=true;
        return sourceSectorImage(std::move(source),geometry,0,{},metadata);
    }
    return std::make_shared<ZonedImage>(kind,std::move(source),*layout,token);
}
MediaFileSystems::FileSystemVolume explore(Kind kind,std::shared_ptr<const MediaFileSystems::RandomAccessData> source,std::stop_token token) {
    const auto image=read(kind,std::move(source),token);auto volume=MediaFileSystems::CommodoreDos::read(*image,token);
    MediaFileSystems::analyzeEntries(formatId(kind),volume,token);return volume;
}
}
