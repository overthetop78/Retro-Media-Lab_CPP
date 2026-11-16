#pragma once
#include "Export.h"
#include "MediaKind.h"
#include "RepresentationInfo.h"
#include <cstdint>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <stop_token>
#include <string>
#include <string_view>
#include <vector>
namespace RetroMediaLab::MediaEngine {
struct PhysicalReadingState;
using PhysicalMetadata=std::map<std::string,std::string>;
class PhysicalDataUnit final {
public:
 RETROMEDIALAB_MEDIAENGINE_API PhysicalDataUnit(std::int64_t,std::vector<std::uint8_t>,PhysicalMetadata={});
 std::int64_t position()const noexcept{return position_;}
 std::span<const std::uint8_t> data()const noexcept{return *data_;}
 const PhysicalMetadata& metadata()const noexcept{return metadata_;}
private:
 std::int64_t position_;std::shared_ptr<const std::vector<std::uint8_t>> data_;PhysicalMetadata metadata_;
};
class PhysicalAcquisitionResult final {
public:
 RETROMEDIALAB_MEDIAENGINE_API PhysicalAcquisitionResult(MediaKind,RepresentationKind,std::vector<PhysicalDataUnit>,PhysicalMetadata={},std::vector<std::string> hardwareDiagnostics={});
 MediaKind mediaKind()const noexcept{return kind_;}
 RepresentationKind representationKind()const noexcept{return representation_;}
 std::span<const PhysicalDataUnit> dataUnits()const noexcept{return units_;}
 const PhysicalMetadata& metadata()const noexcept{return metadata_;}
 std::span<const std::string> hardwareDiagnostics()const noexcept{return diagnostics_;}
private:
 MediaKind kind_;RepresentationKind representation_;std::vector<PhysicalDataUnit> units_;PhysicalMetadata metadata_;std::vector<std::string> diagnostics_;
};
class PhysicalAcquisitionProgress final {
public:
 RETROMEDIALAB_MEDIAENGINE_API PhysicalAcquisitionProgress(int completed,int total,int attempt,std::optional<PhysicalDataUnit> unit={},PhysicalMetadata={},std::shared_ptr<const PhysicalReadingState> readingState={});
 int completedUnits()const noexcept{return completed_;}int totalUnits()const noexcept{return total_;}int attempt()const noexcept{return attempt_;}
 const std::optional<PhysicalDataUnit>& acquiredUnit()const noexcept{return unit_;}
 const PhysicalMetadata& metadata()const noexcept{return metadata_;}
 const std::shared_ptr<const PhysicalReadingState>& readingState()const noexcept{return readingState_;}
private:
 int completed_,total_,attempt_;std::optional<PhysicalDataUnit> unit_;PhysicalMetadata metadata_;
 std::shared_ptr<const PhysicalReadingState> readingState_;
};
class PhysicalWritePlan final {
public:
 RETROMEDIALAB_MEDIAENGINE_API PhysicalWritePlan(MediaKind,RepresentationKind,std::vector<PhysicalDataUnit>,std::vector<int> writeOrder,PhysicalMetadata constraints={});
 MediaKind mediaKind()const noexcept{return kind_;}RepresentationKind representationKind()const noexcept{return representation_;}
 std::span<const PhysicalDataUnit> dataUnits()const noexcept{return units_;}std::span<const int> writeOrder()const noexcept{return order_;}
 const PhysicalMetadata& constraints()const noexcept{return constraints_;}
private:
 MediaKind kind_;RepresentationKind representation_;std::vector<PhysicalDataUnit> units_;std::vector<int> order_;PhysicalMetadata constraints_;
};
class PhysicalWriteProgress final {
public:
 RETROMEDIALAB_MEDIAENGINE_API PhysicalWriteProgress(int completed,int total,std::int64_t position,bool verifying);
 int completedUnits()const noexcept{return completed_;}int totalUnits()const noexcept{return total_;}
 std::int64_t position()const noexcept{return position_;}bool verifying()const noexcept{return verifying_;}
private:
 int completed_,total_;std::int64_t position_;bool verifying_;
};
class PhysicalWriteResult final {
public:
 RETROMEDIALAB_MEDIAENGINE_API PhysicalWriteResult(int completed,int total,bool cancelled,std::vector<std::string> diagnostics={});
 int completedUnits()const noexcept{return completed_;}int totalUnits()const noexcept{return total_;}bool cancelled()const noexcept{return cancelled_;}
 std::span<const std::string> diagnostics()const noexcept{return diagnostics_;}
 bool isSuccess()const noexcept{return !cancelled_&&completed_==total_&&diagnostics_.empty();}
private:
 int completed_,total_;bool cancelled_;std::vector<std::string> diagnostics_;
};
using AcquisitionProgress=std::function<void(const PhysicalAcquisitionProgress&)>;
using PhysicalWritingProgress=std::function<void(const PhysicalWriteProgress&)>;
// Implemented by device providers; these contracts expose no host API or GUI type.
class PhysicalAcquisitionProvider {
public:
 virtual ~PhysicalAcquisitionProvider()=default;
 virtual std::string_view id()const noexcept=0;
 virtual std::span<const MediaKind> supportedMediaKinds()const noexcept=0;
 virtual std::span<const RepresentationKind> outputRepresentationKinds()const noexcept=0;
 virtual bool canAcquire(MediaKind,std::string_view deviceId)const=0;
 virtual std::future<PhysicalAcquisitionResult> acquireAsync(MediaKind,std::string deviceId,PhysicalMetadata options,AcquisitionProgress={},std::stop_token={})const=0;
};
class PhysicalMediaWriter {
public:
 virtual ~PhysicalMediaWriter()=default;
 virtual std::string_view id()const noexcept=0;
 virtual std::span<const MediaKind> supportedMediaKinds()const noexcept=0;
 virtual bool canWrite(const PhysicalWritePlan&,std::string_view deviceId)const=0;
 virtual std::future<PhysicalWriteResult> writeAsync(PhysicalWritePlan,std::string deviceId,PhysicalMetadata options,PhysicalWritingProgress={},std::stop_token={})const=0;
};
}
