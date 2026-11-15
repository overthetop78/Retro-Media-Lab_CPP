#include "FormatRecognition.h"
#include "../../Exploration/Composition/Internal.h"
#include <algorithm>
namespace RetroMediaLab::MediaFileSystems::AtariMetadata {
bool supported(std::string_view format){return std::any_of(Formats.begin(),Formats.end(),[&](auto candidate){return Composition::ordinalKey(format)==Composition::ordinalKey(candidate);});}
}
