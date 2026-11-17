#include <RetroMediaLab/MediaEngine/SectorVisualization.h>
#include "Providers.h"
#include "Constants.h"
#include "../Models/Sectors/Representation.h"
#include <algorithm>
#include <map>
#include <tuple>
namespace RetroMediaLab::MediaEngine {
SectorVisualizationModel describeSectorVisualization(const DiskFormatInfo& format,bool variable,bool logical){
    if((!logical && (!format.heads || !format.sectorsPerTrack || !*format.sectorsPerTrack)) || (!variable && (!format.sectorSize || !*format.sectorSize)))throw MediaError(DiagnosticCode::VisualizationValue,{std::string(Visualization::Constants::Descriptor)});
    SectorVisualizationModel result{logical,logical?1:format.cylinders,logical?1:format.heads,{}};
    using Address=std::tuple<std::int64_t,std::int64_t,std::int64_t>;
    std::map<Address,std::size_t> addresses;
    for(std::size_t index=0;index<format.sectors.size();++index){
        const auto& sector=format.sectors[index];
        const auto track=logical?0:sector.logicalBlock/(*format.sectorsPerTrack);
        const auto cylinder=logical?0:static_cast<std::int64_t>(track/format.heads);
        const auto head=logical?0:static_cast<std::int64_t>(track%format.heads);
        if(!logical && cylinder>=format.cylinders)continue;
        result.sectors.push_back({index,sector.logicalBlock,cylinder,head,sector.number});
        if(sector.state!=DiskSectorState::Missing)++addresses[{cylinder,head,sector.number}];
    }
    std::sort(result.sectors.begin(),result.sectors.end(),[](const auto& left,const auto& right){return left.logicalBlock<right.logicalBlock;});
    for(auto& item:result.sectors){
        const auto& sector=format.sectors[item.sourceIndex];
        if(sector.state==DiskSectorState::Missing)item.state=SectorVisualizationState::WithoutData;
        else if(sector.dataValid==false)item.state=SectorVisualizationState::Dead;
        else if(!sector.data || sector.data->empty())item.state=SectorVisualizationState::WithoutData;
        else if(!logical && (sector.cylinder!=item.cylinder || sector.head!=item.head || addresses[{item.cylinder,item.head,sector.number}]>1))item.state=SectorVisualizationState::Degraded;
        else if(!sector.dataValid || (!variable && sector.data->size()!=*format.sectorSize))item.state=SectorVisualizationState::Degraded;
        else item.state=SectorVisualizationState::WithData;
    }return result;
}
SectorVisualizationModel describeSectorVisualization(std::shared_ptr<const MediaDocument> document,const DiskFormatInfo& format){
    if(!document)throw MediaError(DiagnosticCode::ReaderNullDocument);
    const auto& image=Visualization::representation<SectorRepresentation>(*document).image();
    return describeSectorVisualization(format,image.allowsVariableBlockSize(),image.geometry().addressing==MediaFileSystems::SectorAddressingKind::Logical);
}
}
