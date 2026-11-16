#include <RetroMediaLab/MediaEngine/PhysicalMedia.h>
#include <RetroMediaLab/MediaEngine/PhysicalMediaIds.h>
#include <RetroMediaLab/MediaEngine/FluxTrackData.h>
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include "Constants.h"
#include "../Formats/Scp/Adapter.h"
#include "../Formats/Scp/ProtectedImage.h"
#include "../Formats/Tape/Common/Pcm.h"
#include "../Images/DocumentFactory.h"
#include "../Exploration/Disk/Projection.h"
#include "../Primitives/Identifiers.h"
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaEngine {
namespace P=PhysicalMediaIds;namespace C=PhysicalDetails;namespace S=Scp::Constants;
namespace {
[[noreturn]] void invalid(std::string_view field){throw MediaError(DiagnosticCode::PhysicalContract,{std::string(field)});}
QString metadata(const PhysicalMetadata& values,std::string_view key){const auto it=values.find(std::string(key));if(it==values.end())throw MediaError(DiagnosticCode::PhysicalMetadata,{std::string(key)});return Primitives::utf8Identifier(it->second);}
int integer(const PhysicalMetadata& values,std::string_view key){bool ok=false;const auto value=metadata(values,key).toInt(&ok,C::DecimalBase);if(!ok)throw MediaError(DiagnosticCode::PhysicalMetadata,{std::string(key)});return value;}
std::uint8_t diskType(const PhysicalMetadata& values){const auto text=metadata(values,P::DiskType).trimmed();bool numeric=false;const auto number=text.toUInt(&numeric,C::DecimalBase);if(numeric&&number<=S::ByteMaximum)return static_cast<std::uint8_t>(number);
 std::uint8_t result=0;for(const auto& name:text.split(QChar(C::NameSeparator))){bool found=false;const auto trimmed=name.trimmed().toStdString();for(const auto& [key,value]:C::DiskTypes)if(Primitives::asciiIdentifierEqualsIgnoreCase(trimmed,key)){result|=value;found=true;break;}if(!found)throw MediaError(DiagnosticCode::PhysicalMetadata,{std::string(P::DiskType)});}return result;
}
Scp::Track track(const PhysicalDataUnit& unit,std::stop_token token){
 checkCancellation(token);if(metadata(unit.metadata(),P::Encoding).toStdString()!=P::FluxTrack)throw MediaError(DiagnosticCode::PhysicalMetadata,{std::string(P::Encoding)});
 const auto cylinder=integer(unit.metadata(),P::Cylinder),head=integer(unit.metadata(),P::Head);if(cylinder<0||head<0||head>=int(S::HeadCount)||std::uint64_t(cylinder)*S::HeadCount+head>=S::TrackSlots)invalid(C::PositionField);
 Scp::Track output;output.cylinder=cylinder;output.head=head;output.trackNumber=static_cast<std::uint8_t>(cylinder*S::HeadCount+head);
 const auto data=deserializeFluxTrack(unit.data());for(const auto& source:data.revolutions()){
  Scp::Revolution revolution;const auto rounded=TapePcm::roundEven(double(source.indexTimeNanoseconds())/S::ResolutionStep);revolution.flux.indexTimeTicks=static_cast<std::uint32_t>(std::max(double(S::MinimumTick),rounded));
  std::uint64_t elapsed=0,previous=0;revolution.flux.intervals.reserve(source.intervalsNanoseconds().size());
  for(auto ns:source.intervalsNanoseconds()){checkCancellation(token);elapsed+=ns;const auto converted=static_cast<std::uint64_t>(TapePcm::roundEven(double(elapsed)/S::ResolutionStep));if(converted<=previous)throw MediaError(DiagnosticCode::PhysicalFluxResolution);if(converted-previous>S::MaximumInterval)invalid(C::IntervalField);revolution.flux.intervals.push_back(static_cast<std::uint32_t>(converted-previous));previous=converted;}
  revolution.declaredFluxCount=static_cast<std::uint32_t>(revolution.flux.intervals.size());output.revolutions.push_back(std::move(revolution));
 }return output;
}
Scp::SharedImage image(const PhysicalAcquisitionResult& acquisition,std::stop_token token){
 checkCancellation(token);if(acquisition.mediaKind()!=MediaKind::Floppy||acquisition.representationKind()!=RepresentationKind::Flux)invalid(C::KindField);if(acquisition.dataUnits().empty())invalid(C::UnitsField);
 auto result=std::make_shared<Scp::Image>();for(const auto& unit:acquisition.dataUnits())result->tracks.push_back(track(unit,token));std::sort(result->tracks.begin(),result->tracks.end(),[](const auto& a,const auto& b){return a.trackNumber<b.trackNumber;});
 const auto count=result->tracks.front().revolutions.size();if(count<S::MinimumRevolutions||count>S::MaximumRevolutions)invalid(P::RevolutionCount);bool head0=false,head1=false;int previous=-1;
 for(const auto& current:result->tracks){if(current.trackNumber==previous||current.revolutions.size()!=count)invalid(C::UnitsField);previous=current.trackNumber;head0|=current.head==0;head1|=current.head==1;}
 auto& header=result->header;header.version=S::CaptureVersion;header.diskType=diskType(acquisition.metadata());header.revolutions=static_cast<std::uint8_t>(count);header.startTrack=result->tracks.front().trackNumber;header.endTrack=result->tracks.back().trackNumber;header.flags=S::IndexAlignedFlag|S::WritableFlag|S::ThirdPartyFlag;header.bitCellEncoding=S::DefaultWidth;header.heads=head0&&head1?S::BothHeads:head0?S::Side0:S::Side1;header.resolution=S::CaptureResolution;header.checksum=S::MissingChecksum;return result;
}
}
DiskTrackInfo createAcquiredFloppyTrack(const PhysicalDataUnit& unit,std::stop_token token){auto value=std::make_shared<Scp::Image>();value->tracks.push_back(track(unit,token));value->header.resolution=S::CaptureResolution;return DiskProjectionAccess::capture(value)->tracks.front();}
std::shared_ptr<const DiskCaptureSnapshot> createAcquiredFloppyCapture(const PhysicalAcquisitionResult& value,std::stop_token token){return DiskProjectionAccess::capture(image(value,token));}
PreparedFloppyAcquisition prepareFloppyAcquisition(const std::filesystem::path& path,const PhysicalAcquisitionResult& value,std::stop_token token){const auto encoded=path.u8string();std::string sourcePath(encoded.begin(),encoded.end());if(Primitives::isBlankIdentifier(sourcePath))invalid(C::PathField);auto capture=image(value,token);SourceOptions source;source.primaryPath=std::move(sourcePath);auto document=floppyFluxDocument(std::make_shared<MediaSource>(std::move(source)),std::string(S::FormatId),std::make_shared<Flux::ProtectedTrackImage>(Scp::protectedImage(capture,token)),Scp::metadata(*capture));return {std::move(document),DiskProjectionAccess::capture(std::move(capture))};}
}
