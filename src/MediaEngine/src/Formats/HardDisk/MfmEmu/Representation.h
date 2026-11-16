#pragma once
#include "Container.h"
#include "../Adaptec/SectorReader.h"
#include "../../../Images/MediaDocument.h"
#include <mutex>
namespace RetroMediaLab::MediaEngine::MfmEmu {
class Representation final:public MediaFileSystems::BlockRepresentation,public RepresentationInfo {
public:
    RETROMEDIALAB_MEDIAENGINE_API explicit Representation(std::shared_ptr<const Container>);
    RETROMEDIALAB_MEDIAENGINE_API static std::shared_ptr<const Representation> createUnformatted(std::int32_t cylinders,std::int32_t heads,std::int32_t trackBytes,std::uint32_t rate,std::uint32_t start=0);
    RepresentationKind representationKind()const noexcept override{return RepresentationKind::Flux;}
    RETROMEDIALAB_MEDIAENGINE_API std::optional<std::int64_t> logicalLength()const override;
    RETROMEDIALAB_MEDIAENGINE_API std::int64_t capacity()const override;
    RETROMEDIALAB_MEDIAENGINE_API std::int32_t logicalBlockSize()const override;
    RETROMEDIALAB_MEDIAENGINE_API std::int64_t logicalBlockCount()const override;
    bool supportsRandomAccess()const override{return true;}bool supportsSequentialAccess()const override{return true;}
    RETROMEDIALAB_MEDIAENGINE_API std::future<void> readExactlyAsync(std::int64_t,MediaFileSystems::MutableBytes,std::stop_token={})const override;
    RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaDocument> createLogicalView(std::shared_ptr<const MediaDocument>)const;
    RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const Representation> withTrackBytes(std::int32_t cylinder,std::int32_t head,std::span<const std::uint8_t>)const;
    RETROMEDIALAB_MEDIAENGINE_API Flux::NanosecondRevolution readTrack(std::int32_t cylinder,std::int32_t head,std::uint32_t period,std::stop_token={})const;
    RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const Representation> withTrackWindow(std::int32_t cylinder,std::int32_t head,std::uint32_t period,std::uint32_t start,std::uint32_t duration,std::span<const std::uint32_t>,bool includeEndingInstant=false,std::stop_token={})const;
    void readTrackBytes(std::int32_t c,std::int32_t h,std::int32_t offset,MediaFileSystems::MutableBytes destination,std::stop_token token={})const{container_->readTrackBytes(c,h,offset,destination,token);}
    std::int32_t cylinders()const noexcept{return container_->header().cylinders();}std::int32_t heads()const noexcept{return container_->header().heads();}
    std::int32_t trackByteLength()const noexcept{return container_->header().trackDataLength();}
    std::uint32_t sampleRate()const noexcept{return container_->header().sampleRate();}std::uint32_t startNanoseconds()const noexcept{return container_->header().startNanoseconds();}
    const std::shared_ptr<const Container>& container()const noexcept{return container_;}
private:
    std::shared_ptr<Adaptec::SectorReader> sectors()const;
    std::shared_ptr<const Container> container_;mutable std::mutex mutex_;mutable std::shared_ptr<Adaptec::SectorReader> sectors_;
};
}
