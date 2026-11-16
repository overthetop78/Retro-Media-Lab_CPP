#pragma once
#include "../../Reconstruction/Scp/Composition.h"
#include "../../Composition/Writing.h"
#include <RetroMediaLab/MediaEngine/MediaFileConversion.h>
namespace RetroMediaLab::MediaEngine::SectorFileConversion {
enum class Family {Ibm,Msx,BbcDfs,AcornAdf,AmigaAdf,Amstrad,Epson,Ucsd,Rx02,D81,AtariSt,Atr,AppleSector,AppleNibble,Macintosh,Lisa,CommodoreDos,Coherent};
RETROMEDIALAB_MEDIAENGINE_API std::optional<Family> family(std::string_view);
class Service final {
public:
 RETROMEDIALAB_MEDIAENGINE_API Service(ScpReconstruction::Composition,MediaWritingComposition);
 RETROMEDIALAB_MEDIAENGINE_API bool canCreate(std::string_view,std::string_view)const;
 RETROMEDIALAB_MEDIAENGINE_API std::future<std::vector<std::filesystem::path>> convertAsync(std::filesystem::path,std::filesystem::path,std::string,std::stop_token={})const;
private:
 std::shared_ptr<const MediaDocument> readApple(const std::filesystem::path&,std::string_view,Family,std::stop_token)const;
 std::shared_ptr<const MediaDocument> read(const std::filesystem::path&,std::string_view,Family,std::stop_token)const;
 ScpReconstruction::Composition scp_;MediaWritingComposition writing_;
};
}
