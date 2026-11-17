#include <RetroMediaLab/MediaEngine/FluxVisualization.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <limits>
#include "Constants.h"
#include "../Decoding/Registry.h"
#include "../Decoding/Scoring.h"
#include <algorithm>
#include <numeric>
#include <cmath>
namespace RetroMediaLab::MediaEngine {
namespace {
using namespace Visualization::Constants;
double middle(std::vector<double> values){if(values.empty())return 0;const auto center=values.begin()+values.size()/FluxMedianDivisor;std::nth_element(values.begin(),center,values.end());return *center;}
FluxVisualRevolution prepare(const DiskRevolutionInfo& source,std::stop_token token){
    FluxVisualRevolution value;value.number=source.number;value.milliseconds=source.durationNanoseconds/NanosecondsPerMillisecond;value.rpm=source.durationNanoseconds>0?NanosecondsPerMinute/source.durationNanoseconds:0;
    if(!source.fluxIntervals || source.fluxIntervals->empty())return value;const auto& intervals=*source.fluxIntervals;value.transitions=intervals.size();const auto total=std::accumulate(intervals.begin(),intervals.end(),double{});if(total<=0)return value;
    auto ordered=intervals;const auto center=ordered.begin()+ordered.size()/FluxMedianDivisor;std::nth_element(ordered.begin(),center,ordered.end());const auto median=*center;const auto step=std::max<std::size_t>(1,intervals.size()/FluxArcSamples);double elapsed=0;
    for(std::size_t index=0;index<intervals.size();index+=step){checkCancellation(token);const auto end=std::min(intervals.size(),index+step);const auto span=std::accumulate(intervals.begin()+index,intervals.begin()+end,double{});const auto mark=intervals[index]<median*FluxShortRatio?FluxVisualMark::Short:intervals[index]>median*FluxLongRatio?FluxVisualMark::Long:FluxVisualMark::Normal;value.arcs.push_back({elapsed/total*FluxCircle-FluxTop,std::max(FluxMinimumSweep,span/total*FluxCircle),mark});elapsed+=span;}
    return value;
}
FluxVisualMark structureMark(Decoding::StructureKind kind){using enum Decoding::StructureKind;switch(kind){case IdAddressMark:case AppleAddress:case CommodoreHeader:case FormatHeader:return FluxVisualMark::Header;case DataAddressMark:case AppleData:case FormatData:return FluxVisualMark::Data;case DeletedDataAddressMark:return FluxVisualMark::Deleted;case TimingAnomaly:return FluxVisualMark::Timing;default:return FluxVisualMark::Other;}}
}
FluxVisualizationModel describeFluxVisualization(const DiskImageInfo& disk,std::stop_token token){
    FluxVisualizationModel result;const Decoding::Registry registry;for(const auto& decoder:registry.decoders())result.decoders.emplace_back(decoder->id());
    for(const auto& source:disk.tracks){checkCancellation(token);FluxVisualTrack track;track.head=source.head;track.cylinder=source.cylinder;track.sourceNumber=source.sourceNumber;std::vector<double> durations,counts;
        for(const auto& revolution:source.revolutions){track.revolutions.push_back(prepare(revolution,token));durations.push_back(static_cast<double>(revolution.durationNanoseconds));counts.push_back(static_cast<double>(track.revolutions.back().transitions));}
        const auto medianDuration=middle(std::move(durations)),medianCount=middle(std::move(counts));
        std::size_t samples=0;for(std::size_t index=0;index<track.revolutions.size();++index){auto& value=track.revolutions[index];if(track.revolutions.size()==1)value.quality=value.transitions?1:0;else if(value.transitions && medianDuration>0 && medianCount>0){const auto durationDeviation=std::abs(static_cast<double>(source.revolutions[index].durationNanoseconds)-medianDuration)/medianDuration;const auto countDeviation=std::abs(static_cast<double>(value.transitions)-medianCount)/medianCount;value.quality=std::clamp(1-durationDeviation/FluxDurationTolerance,FluxMinimumQuality,FluxMaximumQuality)*FluxDurationWeight+std::clamp(1-countDeviation/FluxCountTolerance,FluxMinimumQuality,FluxMaximumQuality)*FluxCountWeight;}samples=std::max(samples,value.arcs.size());track.synthesis.quality+=value.quality;}
        if(!track.revolutions.empty())track.synthesis.quality/=track.revolutions.size();
        for(std::size_t sample=0;sample<samples;++sample){checkCancellation(token);std::vector<FluxVisualMark> order;std::vector<std::size_t> votes;for(const auto& revolution:track.revolutions)if(!revolution.arcs.empty()){const auto mark=revolution.arcs[std::min(revolution.arcs.size()-1,sample*revolution.arcs.size()/samples)].mark;const auto found=std::find(order.begin(),order.end(),mark);if(found==order.end()){order.push_back(mark);votes.push_back(1);}else ++votes[found-order.begin()];}const auto best=static_cast<std::size_t>(std::max_element(votes.begin(),votes.end())-votes.begin());track.synthesis.arcs.push_back({-FluxTop+sample*FluxCircle/samples,FluxCircle/samples,order[best]});}
        result.tracks.push_back(std::move(track));
    }return result;
}
FluxTrackInspection inspectFluxVisualization(const DiskTrackInfo& track,std::optional<std::string> decoder,std::stop_token token){
    Decoding::Registry registry;std::vector<Flux::SharedRevolution> revolutions;std::vector<std::size_t> indices;
    for(std::size_t index=0;index<track.revolutions.size();++index){checkCancellation(token);const auto& source=track.revolutions[index];if(!source.fluxIntervals || source.fluxIntervals->empty())continue;auto value=std::make_shared<Flux::Revolution>();value->intervals=*source.fluxIntervals;const auto total=std::accumulate(value->intervals.begin(),value->intervals.end(),double{});const auto period=source.resolutionNanoseconds>0?static_cast<double>(source.durationNanoseconds)/source.resolutionNanoseconds:total;if(period<0 || period>std::numeric_limits<std::uint32_t>::max())throw MediaError(DiagnosticCode::ScpOverflow);value->indexTimeTicks=static_cast<std::uint32_t>(period);indices.push_back(index);revolutions.push_back(std::move(value));}
    FluxTrackInspection output;if(revolutions.empty())return output;std::optional<Decoding::Selection> best;
    for(std::size_t index=0;index<revolutions.size();++index){Decoding::SharedResult candidate;
        if(decoder){checkCancellation(token);candidate=registry.decode(*decoder,revolutions[index]);}
        else for(const auto& entry:registry.decoders())if(entry->participatesInAutomaticDetection()){checkCancellation(token);const auto value=registry.decode(entry->id(),revolutions[index]);if(!candidate || Decoding::betterAutomatic(*value,*candidate))candidate=value;}
        if(candidate && (!best || (decoder?Decoding::betterExplicit(*candidate,*best->result):Decoding::betterAutomatic(*candidate,*best->result))))best=Decoding::Selection{index,candidate};
    }
    checkCancellation(token);if(!best)return output;const auto& decoded=*best->result;output.decoderId=decoded.decoderId;output.confidence=decoded.confidence;output.cellTicks=decoded.estimatedCellTicks;output.revolution=indices[best->revolutionIndex];const auto& intervals=revolutions[best->revolutionIndex]->intervals;const auto total=std::accumulate(intervals.begin(),intervals.end(),double{});const auto bits=decoded.estimatedCellTicks>0?std::max(FluxMaximumQuality,total/decoded.estimatedCellTicks):0;
    for(const auto& structure:decoded.structures){checkCancellation(token);output.details.push_back({FluxStructureKeys[static_cast<std::size_t>(structure.kind)],structure.bitOffset,structure.bitLength});if(bits>0)output.structures.push_back({structure.bitOffset/bits*FluxCircle-FluxTop,std::max(FluxStructureMinimumSweep,structure.bitLength/bits*FluxCircle),structureMark(structure.kind)});}
    for(std::size_t index=0;index<decoded.sectors.size();++index){const auto& source=decoded.sectors[index];DiskSectorInfo sector;sector.logicalBlock=index;sector.cylinder=source.cylinder;sector.head=source.head;sector.number=source.number;sector.size=source.sizeBytes;sector.dataValid=source.integrityValid;sector.headerIntegrityValid=source.headerIntegrityValid;sector.formatCode=source.formatCode;sector.state=source.data?(source.integrityValid==false?DiskSectorState::Invalid:DiskSectorState::Available):DiskSectorState::Missing;if(source.data)sector.data=std::make_shared<const std::vector<std::uint8_t>>(*source.data);if(source.tag)sector.tag=std::make_shared<const std::vector<std::uint8_t>>(*source.tag);output.sectors.push_back(std::move(sector));}
    return output;
}
}
