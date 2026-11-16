#pragma once
#include "Recognition.h"
#include "../Reconstruction/Scp/Composition.h"
#include "../Images/Conversion/Representation.h"
namespace RetroMediaLab::MediaEngine {
RETROMEDIALAB_MEDIAENGINE_API RepresentationConverterRegistry createRepresentationConverters(const MediaRecognitionComposition&,const ScpReconstruction::Composition&);
}
