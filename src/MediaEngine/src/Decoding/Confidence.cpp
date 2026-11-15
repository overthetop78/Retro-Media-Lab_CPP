#include "Confidence.h"
#include "Constants.h"
#include <cmath>
#include <bit>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Decoding {
namespace {double capped(double value){return std::isnan(value)?value:std::min(Constants::MaximumConfidence,value);}}
double confidence(std::int32_t sectors,std::int32_t structures,std::int32_t weight,double divisor){const auto wrapped=std::uint32_t(sectors)*std::uint32_t(weight)+std::uint32_t(structures);return capped(std::bit_cast<std::int32_t>(wrapped)/divisor);}
double standardConfidence(std::int32_t sectors,std::int32_t structures){return confidence(sectors,structures,Constants::SectorWeight,Constants::StandardDivisor);}
double validityConfidence(std::int32_t valid,std::int32_t detected,double a,double b){return detected==0?0:capped(valid/a+detected/b);}
}
