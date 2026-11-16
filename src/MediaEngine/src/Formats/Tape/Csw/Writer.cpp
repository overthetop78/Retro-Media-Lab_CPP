#include "Reader.h"
#include "Constants.h"
#include "Compression.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <array>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Csw {
using namespace Constants;
const WriterCapabilities& Writer::capabilities()const{static const WriterCapabilities caps{std::string(WriterId),{std::string(FormatId)},{std::string(Extension)},{RepresentationKind::Sequential},false};return caps;}
bool Writer::canWrite(const MediaDocument& document,std::string_view format,std::string_view extension)const{int channels=0,rate=0,bits=0,align=0;return document.mediaKind()==MediaKind::Tape&&document.representationInfo().representationKind()==RepresentationKind::Sequential&&format==FormatId&&sameIdentifier(extension,Extension)&&TapePcm::tryProfile(document.metadata(),channels,rate,bits,align);}
std::future<std::vector<std::filesystem::path>> Writer::writeAsync(std::shared_ptr<const MediaDocument> document,std::filesystem::path path,std::string format,std::stop_token token)const{return std::async(std::launch::deferred,[document=std::move(document),path=std::move(path),format=std::move(format),token]{
    checkCancellation(token);if(!document)throw MediaError(DiagnosticCode::HardDiskRawNullDocument);const auto extension=path.extension().u8string();if(!Writer{}.canWrite(*document,format,std::string(extension.begin(),extension.end())))throw MediaError(DiagnosticCode::CswWrite);
    const auto native=TapePcm::readChannels(*document,token);const auto& samples=native.channels.front();bool high=!samples.empty()&&samples.front()>=0;std::uint32_t length{},count{};std::vector<std::uint8_t> pulses;
    const auto pulse=[&]{if(length<=MaximumShortPulse)pulses.push_back(static_cast<std::uint8_t>(length));else{pulses.push_back(0);const auto offset=pulses.size();pulses.resize(offset+ExtendedBytes);qToLittleEndian(length,pulses.data()+offset);}length=0;++count;};
    for(const auto sample:samples){checkCancellation(token);const bool current=sample==0?high:sample>0;if(current!=high&&length){pulse();high=current;}++length;}if(length)pulse();
    std::array<std::uint8_t,HeaderV2> header{};std::copy(Signature.begin(),Signature.end(),header.begin());header[TerminatorOffset]=Terminator;header[MajorOffset]=Version2;qToLittleEndian(static_cast<std::uint32_t>(native.rate),header.data()+RateOffset);qToLittleEndian(count,header.data()+CountOffset);header[CompressionV2]=Zrle;header[FlagsV2]=!samples.empty()&&samples.front()>=0?HighFlag:0;std::copy(Application.begin(),Application.end(),header.begin()+ApplicationOffset);
    writeAtomicImage(path,[&](const auto& output,auto stop){checkCancellation(stop);output(header);deflatePulses(pulses,output,stop);},token);return std::vector<std::filesystem::path>{path};
});}
}
