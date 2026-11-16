#pragma once
#include "Recognition.h"
#include "Writing.h"
#include "Conversion.h"
#include "../Images/Conversion/Service.h"
#include "../Images/Conversion/Files.h"
#include "../Images/Conversion/Optical.h"
#include "../Images/Conversion/Sequential/Service.h"
#include "../Exploration/Scp/Inspection.h"
#include "../Exploration/Disk/Explorer.h"
#include "../Encoding/SectorImageEncoder.h"
#include "../Reconstruction/Scp/Composition.h"
#include "../Formats/Scp/Adapter.h"
#include "../Exploration/Media/Service.h"
#include "../Visualization/Providers.h"
namespace RetroMediaLab::MediaEngine::RuntimeDetails {
struct Services {
    MediaRecognitionComposition reading=MediaRecognitionComposition::createDefault();
    MediaWritingComposition writing=MediaWritingComposition::createDefault(reading.driveService());
    MediaFileSystems::SectorRegistry fileSystems;
    SectorInterpretations interpretations=SectorInterpretations::createDefault({fileSystems.supportedFormatIds().begin(),fileSystems.supportedFormatIds().end()});
    ScpReconstruction::Composition sectors{reading.scpReader(),fileSystems};
    RepresentationConverterRegistry conversions=createRepresentationConverters(reading,sectors);
    MediaConversionService conversion{conversions,writing.registry(),writing.writingService()};
    SectorFileConversion::Service sectorFiles{sectors,writing};
    ScpInspection::Service scpInspection{reading.scpReader(),sectors.candidates(),sectors.decodeOperation(),ScpInspection::Inspector(fileSystems,interpretations)};
    DiskImageExplorer disks{reading.readingService(),fileSystems,scpInspection,sectors.reader(),interpretations};
    MediaExplorer media=MediaExplorer::createDefault();
    MediaExplorationService exploration{reading.readingService(),disks,media};
    Visualization::Registry visualization=Visualization::Registry::createDefault();
    Encoding::SectorImageEncoder sectorEncoder;
    OpticalImageConversionService optical{reading.readingService(),writing.registry(),writing.writingService()};
    SequentialMediaConversionService sequential{reading.readingService(),SequentialMediaComposition::createDefault(),writing.registry(),writing.writingService()};
};
std::shared_ptr<Services> defaultServices();
}
