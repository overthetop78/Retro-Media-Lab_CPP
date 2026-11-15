#include "ImageDisk.h"
#include "Constants.h"
#include "../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QString>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::ImageDisk {
namespace {
using namespace Constants;
void appendComment(std::vector<std::uint8_t>& bytes,const std::string& value){const auto text=QString::fromUtf8(value);for(qsizetype i=0;i<text.size();++i){const auto character=text[i];if(character.isHighSurrogate()&&i+1<text.size()&&text[i+1].isLowSurrogate())++i;bytes.push_back(character.unicode()<=AsciiMaximum?std::uint8_t(character.unicode()):AsciiReplacement);}}
void writeTrack(std::vector<std::uint8_t>& bytes,const Track& t){if(std::uint8_t(t.mode)>MaxMode||t.sectors.empty()||t.sectors.size()>MaxByte)throw MediaError(DiagnosticCode::ImdWriteTrack);const bool cylinderMap=std::any_of(t.sectors.begin(),t.sectors.end(),[&](const auto&s){return s.cylinder!=t.cylinder;}),headMap=std::any_of(t.sectors.begin(),t.sectors.end(),[&](const auto&s){return s.head!=t.head;});std::uint8_t flags=t.head&HeadMask;if(cylinderMap)flags|=CylinderMap;if(headMap)flags|=HeadMap;std::optional<std::uint8_t> code;if(std::all_of(t.sectors.begin(),t.sectors.end(),[&](const auto&s){return s.size==t.sectors.front().size;}))for(std::uint8_t candidate=0;candidate<=MaxSizeCode;++candidate)if(std::int32_t(BaseSize<<candidate)==t.sectors.front().size){code=candidate;break;}
 bytes.insert(bytes.end(),{std::uint8_t(t.mode),t.cylinder,flags,std::uint8_t(t.sectors.size()),code.value_or(ExplicitSize)});for(const auto&s:t.sectors)bytes.push_back(s.number);if(cylinderMap)for(const auto&s:t.sectors)bytes.push_back(s.cylinder);if(headMap)for(const auto&s:t.sectors)bytes.push_back(s.head);if(!code)for(const auto&s:t.sectors){if(s.size<0||std::uint32_t(s.size)>MaxSize)throw MediaError(DiagnosticCode::ImdWriteSector);bytes.push_back(std::uint8_t(s.size));bytes.push_back(std::uint8_t(std::uint32_t(s.size)>>BitsPerByte));}
 for(const auto&s:t.sectors){if(std::uint8_t(s.recordType)>MaxRecord||s.size<0||s.data.size()!=std::size_t(s.size))throw MediaError(DiagnosticCode::ImdWriteSector);bytes.push_back(std::uint8_t(s.recordType));if(!hasData(s.recordType))continue;if(compressed(s.recordType)){if(s.data.empty()||!std::all_of(s.data.begin(),s.data.end(),[&](auto value){return value==s.data.front();}))throw MediaError(DiagnosticCode::ImdCompressed);bytes.push_back(s.data.front());}else bytes.insert(bytes.end(),s.data.begin(),s.data.end());}
}
}
std::vector<std::uint8_t> build(const Image& image){using namespace Constants;if(image.tracks.empty())throw MediaError(DiagnosticCode::ImdNoTracks);auto comment=image.comment.empty()?std::string(DefaultComment):image.comment;if(!comment.starts_with(Signature))comment=std::string(CommentPrefix)+comment;std::vector<std::uint8_t> bytes;appendComment(bytes,comment);bytes.push_back(Terminator);for(const auto&t:image.tracks)writeTrack(bytes,t);return bytes;}
void write(const Image& image,const std::filesystem::path& path,std::stop_token token){const auto bytes=build(image);writeAtomicImage(path,[&](const ImageOutput& output,std::stop_token stop){checkCancellation(stop);output(bytes);},token);}
}
