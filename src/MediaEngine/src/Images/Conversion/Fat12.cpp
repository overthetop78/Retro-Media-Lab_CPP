#include "Fat12.h"
#include "../../Formats/Raw/RawImages.h"
#include "../../Formats/Raw/Constants.h"
#include "../../Formats/St/Geometry.h"
#include "../../Formats/St/Writer.h"
#include "../../Models/Sectors/SectorImages.h"
#include "../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaFileSystems/Fat12.h>
#include <RetroMediaLab/MediaFileSystems/FatBootSectorLayout.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Fat12Reinterpretation {
std::optional<MediaFileSystems::SectorGeometry> targetGeometry(std::string_view id){
 if(const auto geometry=St::geometry(id))return geometry;
 for(const auto& geometry:Raw::ibmGeometries())if(Primitives::asciiIdentifierEqualsIgnoreCase(geometry.formatId,id))return geometry;
 for(const auto& geometry:Raw::msxGeometries())if(Primitives::asciiIdentifierEqualsIgnoreCase(geometry.formatId,id))return geometry;
 return {};
}
MediaFileSystems::SectorGeometry validate(const MediaFileSystems::SectorImage& source,std::string_view targetId,bool hybrid,std::stop_token token){
 checkCancellation(token);if(Primitives::isBlankIdentifier(targetId))throw MediaError(DiagnosticCode::ConversionRequest);
 if(hybrid)throw MediaError(DiagnosticCode::Fat12HybridSource);
 const auto target=targetGeometry(targetId);if(!target)throw MediaError(DiagnosticCode::Fat12UnsupportedTarget,{std::string(targetId)});
 const auto& geometry=source.geometry();
 for(std::uint64_t number=0;number<source.blockCount();++number){checkCancellation(token);if(!source.containsBlock(number))throw MediaError(DiagnosticCode::Fat12MissingSectors,{geometry.formatId});}
 bool complete=true;source.visitAvailableBlocks([&](auto number){const auto block=source.block(number,token);complete=block&&block->data.size()==geometry.blockSize;return complete;},token);
 if(!complete)throw MediaError(DiagnosticCode::Fat12MissingSectors,{geometry.formatId});
 const auto total=std::uint64_t(target->cylinders)*target->heads*target->sectorsPerTrack,capacity=total*target->blockSize;
 if(geometry.blockSize!=target->blockSize||geometry.cylinders!=target->cylinders||geometry.heads!=target->heads||geometry.sectorsPerTrack!=target->sectorsPerTrack||source.blockCount()!=total||source.capacity()!=capacity)throw MediaError(DiagnosticCode::Fat12IncompatibleGeometry,{geometry.formatId,std::string(targetId)});
 const auto boot=source.block(MediaFileSystems::FatBootSectorLayout::BootLogicalBlock,token);
 if(!boot)throw MediaError(DiagnosticCode::Fat12InvalidBpb,{geometry.formatId});
 const auto bpb=MediaFileSystems::Fat::detectBpbGeometry(boot->data,capacity);
 if(!bpb||bpb->sectorSize!=target->blockSize||bpb->totalSectors!=total||bpb->cylinders!=target->cylinders||bpb->heads!=target->heads||bpb->sectorsPerTrack!=target->sectorsPerTrack||!MediaFileSystems::Fat12::readLayout(boot->data,source.blockCount(),geometry.formatId))throw MediaError(DiagnosticCode::Fat12InvalidBpb,{geometry.formatId});
 return *target;
}
void convert(std::shared_ptr<const MediaFileSystems::SectorImage> image,const std::filesystem::path& output,std::string target,bool hybrid,std::stop_token token){
 if(!image||output.empty())throw MediaError(DiagnosticCode::ConversionRequest);
 const auto geometry=validate(*image,target,hybrid,token);
 if(!Primitives::asciiIdentifierEqualsIgnoreCase(image->geometry().formatId,target))image=relabelSectorImage(std::move(image),geometry.formatId);
 if(St::geometry(target)){St::write(*image,output,token);return;}
 if(Primitives::asciiIdentifierStartsWithIgnoreCase(target,Raw::Constants::IbmPrefix)){Raw::writeIbm(*image,output,target,token);return;}
 Raw::writeMsx(*image,output,target,token);
}
}
