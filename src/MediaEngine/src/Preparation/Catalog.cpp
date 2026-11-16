#include <RetroMediaLab/MediaEngine/MediaPreparation.h>
#include "CatalogConstants.h"
#include "../Images/Optical/Constants.h"
namespace RetroMediaLab::MediaEngine {
const std::map<std::string,std::string>& mediaFileExtensions(){static const std::map<std::string,std::string> result(PreparationConstants::FileExtensions.begin(),PreparationConstants::FileExtensions.end());return result;}
std::int32_t rawOpticalSectorSize()noexcept{return OpticalSessionConstants::SectorBytes;}
std::int32_t audioOpticalSectorSize()noexcept{return OpticalSessionConstants::SectorBytes;}
std::int32_t opticalSubchannelSize()noexcept{return OpticalSessionConstants::SubBytes;}
}
