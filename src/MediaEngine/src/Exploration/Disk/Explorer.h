#pragma once
#include "../../Images/ReadingService.h"
#include "../../Images/Recognition/Interpretations.h"
#include "../../Reconstruction/Scp/Reader.h"
#include "../Scp/Inspection.h"
#include <RetroMediaLab/MediaEngine/DiskMetadata.h>
namespace RetroMediaLab::MediaEngine {
// Technical disk result. GUI metadata and presentation are projected separately.
struct DiskExploration {
 std::filesystem::path sourcePath;
 std::shared_ptr<const MediaDocument> document;
 SectorImageOwner image;
 std::vector<ScpInspection::MatchOwner> detected;
 std::vector<SectorImageOwner> decodedImages;
 std::string primaryFormatId;
 std::optional<std::string> manualFormatId;
 Scp::SharedImage capture;
 DiskImageMetadata metadata;
 RETROMEDIALAB_MEDIAENGINE_API std::vector<std::string> detectedFormats()const;
 RETROMEDIALAB_MEDIAENGINE_API bool isHybrid()const;
 RETROMEDIALAB_MEDIAENGINE_API std::optional<DiskExploration> selectFormat(std::string_view)const;
};
using DiskCaptureLoaded=std::function<void(Scp::SharedImage)>;
class DiskImageExplorer final {
public:
 RETROMEDIALAB_MEDIAENGINE_API DiskImageExplorer(MediaImageReadingService,MediaFileSystems::SectorRegistry,ScpInspection::Service,ScpReconstruction::Reader,SectorInterpretations);
 RETROMEDIALAB_MEDIAENGINE_API std::shared_ptr<const MediaDocument> readDocument(const std::filesystem::path&,std::optional<std::string> format={},std::stop_token={})const;
 RETROMEDIALAB_MEDIAENGINE_API DiskExploration explore(const std::filesystem::path&,std::optional<std::string> format={},const ScpInspection::Progress& = {},std::stop_token={})const;
 RETROMEDIALAB_MEDIAENGINE_API DiskExploration explore(std::shared_ptr<const MediaDocument>,std::optional<std::string> format={},const ScpInspection::Progress& = {},std::stop_token={},const DiskCaptureLoaded& = {})const;
 RETROMEDIALAB_MEDIAENGINE_API DiskExploration exploreScp(const std::filesystem::path&,Scp::SharedImage,const ScpInspection::Progress& = {},std::stop_token={})const;
private:
 DiskExploration exploreSectors(const std::filesystem::path&,std::shared_ptr<const MediaDocument>,SectorImageOwner,std::optional<std::string>,Scp::SharedImage,std::stop_token)const;
 DiskExploration fromScp(const std::filesystem::path&,std::shared_ptr<const MediaDocument>,ScpInspection::Result,std::stop_token)const;
 MediaImageReadingService reading_;MediaFileSystems::SectorRegistry fileSystems_;ScpInspection::Service scp_;ScpReconstruction::Reader sectors_;SectorInterpretations interpretations_;
};
}
