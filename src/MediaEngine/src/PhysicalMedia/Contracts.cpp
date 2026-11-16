#include <RetroMediaLab/MediaEngine/PhysicalMediaTypes.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include "Constants.h"
#include <algorithm>
#include <set>
namespace RetroMediaLab::MediaEngine {
namespace {
void require(bool valid,std::string_view field){if(!valid)throw MediaError(DiagnosticCode::PhysicalContract,{std::string(field)});}
void progress(int completed,int total,bool allowEmpty){require(completed>=0&&total>=0&&(allowEmpty||total>0)&&completed<=total,PhysicalDetails::ProgressField);}
}
PhysicalDataUnit::PhysicalDataUnit(std::int64_t position,std::vector<std::uint8_t> data,PhysicalMetadata metadata):position_(position),data_(std::make_shared<const std::vector<std::uint8_t>>(std::move(data))),metadata_(std::move(metadata)){require(position>=0,PhysicalDetails::PositionField);}
PhysicalAcquisitionResult::PhysicalAcquisitionResult(MediaKind kind,RepresentationKind representation,std::vector<PhysicalDataUnit> units,PhysicalMetadata metadata,std::vector<std::string> diagnostics):kind_(kind),representation_(representation),units_(std::move(units)),metadata_(std::move(metadata)),diagnostics_(std::move(diagnostics)){std::stable_sort(units_.begin(),units_.end(),[](const auto& a,const auto& b){return a.position()<b.position();});}
PhysicalAcquisitionProgress::PhysicalAcquisitionProgress(int completed,int total,int attempt,std::optional<PhysicalDataUnit> unit,PhysicalMetadata metadata):completed_(completed),total_(total),attempt_(attempt),unit_(std::move(unit)),metadata_(std::move(metadata)){progress(completed,total,false);require(attempt>0,PhysicalDetails::AttemptField);}
PhysicalWritePlan::PhysicalWritePlan(MediaKind kind,RepresentationKind representation,std::vector<PhysicalDataUnit> units,std::vector<int> order,PhysicalMetadata constraints):kind_(kind),representation_(representation),units_(std::move(units)),order_(std::move(order)),constraints_(std::move(constraints)){std::set<int> seen;for(auto index:order_)require(index>=0&&std::size_t(index)<units_.size()&&seen.insert(index).second,PhysicalDetails::OrderField);}
PhysicalWriteProgress::PhysicalWriteProgress(int completed,int total,std::int64_t position,bool verifying):completed_(completed),total_(total),position_(position),verifying_(verifying){progress(completed,total,false);require(position>=0,PhysicalDetails::PositionField);}
PhysicalWriteResult::PhysicalWriteResult(int completed,int total,bool cancelled,std::vector<std::string> diagnostics):completed_(completed),total_(total),cancelled_(cancelled),diagnostics_(std::move(diagnostics)){progress(completed,total,true);}
}
