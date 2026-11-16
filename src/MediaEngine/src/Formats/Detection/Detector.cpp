#include "Detector.h"
#include "Constants.h"
#include "../../Primitives/Identifiers.h"
#include "../../Primitives/Path.h"
#include "../../Sources/RandomAccessSources.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
#include <ios>
namespace RetroMediaLab::MediaEngine::Detection {
const DiskFormat* Context::find(std::string_view id)const{const auto found=std::find_if(formats.begin(),formats.end(),[&](const auto& format){return Primitives::ordinalIdentifierEqualsIgnoreCase(format.id,id);});return found==formats.end()?nullptr:&*found;}
DetectedImageFormat Context::result(std::optional<std::string_view> id,FormatConfidence confidence,std::string_view key)const{const auto format=id?find(*id):nullptr;return {extension,format?std::optional<DiskFormat>(*format):std::nullopt,id?confidence:FormatConfidence::Ambiguous,candidates,{std::string(key),{},std::string(Constants::Domain)}};}
DetectedImageFormat Context::ambiguous(std::string_view key)const{return result({},FormatConfidence::Ambiguous,key);}
Source Context::open()const{checkCancellation(token);auto source=openRead(path);if(!source)throw MediaFileSystems::MediaError(MediaFileSystems::DiagnosticCode::InvalidSource);return source;}
std::vector<std::uint8_t> Context::read(const Source& source,std::uint64_t offset,std::size_t count)const{checkCancellation(token);const auto sourceLength=source->length();std::vector<std::uint8_t> bytes(offset<sourceLength?std::size_t(std::min<std::uint64_t>(count,sourceLength-offset)):0);if(!bytes.empty())source->readExactly(offset,bytes,token);return bytes;}
bool probeFailure(const std::exception_ptr& failure){if(!failure)return false;try{std::rethrow_exception(failure);}catch(const MediaFileSystems::MediaError& error){return error.kind()==MediaFileSystems::ErrorKind::Io||error.kind()==MediaFileSystems::ErrorKind::NotFound;}catch(const std::ios_base::failure&){return true;}catch(...){return false;}}
DetectedImageFormat detect(std::span<const DiskFormat> formats,const std::filesystem::path& path,std::optional<std::int64_t> length,OpenRead open,std::stop_token token){
 checkCancellation(token);Context context{formats,path,length,Primitives::utf8Identifier(Primitives::pathExtension(path)).toLower().toUtf8().toStdString(),{},open?std::move(open):OpenRead(fileSource),token};
 for(const auto& format:formats)if(std::any_of(format.extensions.begin(),format.extensions.end(),[&](const auto& suffix){return Primitives::ordinalIdentifierEqualsIgnoreCase(suffix.extension,context.extension);}))context.candidates.push_back(format);
 using namespace Constants;if(context.extension==Scp)return context.result(RawScp,FormatConfidence::Certain,RawExplanation);
 if(auto found=adf(context))return std::move(*found);if(auto found=atari(context))return std::move(*found);
 if(context.extension==D13)return context.result(Formats::AppleIIAppleDos113,FormatConfidence::Certain,AppleDosOrder);if(context.extension==Do)return context.result(Formats::AppleIIAppleDos140,FormatConfidence::Certain,AppleDosOrder);if(context.extension==Po)return context.result(Formats::AppleIIProDos140,FormatConfidence::Certain,AppleProDosOrder);if(context.extension==TwoMg)return context.ambiguous(AppleContainer);
 if(auto found=macintosh(context))return std::move(*found);
 if(context.extension==Ima||context.extension==Img){const auto id=sizeFormat(IbmSizes,length);return id?context.result(id,FormatConfidence::Certain,IbmSize):context.ambiguous(IbmAmbiguous);}
 if(context.candidates.size()==1)return {context.extension,context.candidates.front(),FormatConfidence::Inferred,context.candidates,{std::string(ExtensionInferred),{},std::string(Domain)}};return context.ambiguous(Multiple);
}
}
namespace RetroMediaLab::MediaEngine {
ImageFormatDetector::ImageFormatDetector():formats_(imageFormats().begin(),imageFormats().end()){}
ImageFormatDetector::ImageFormatDetector(std::vector<DiskFormat> formats):formats_(std::move(formats)){}
DetectedImageFormat ImageFormatDetector::detect(const std::filesystem::path& path,std::optional<std::int64_t> length,std::stop_token token)const{return Detection::detect(formats_,path,length,{},token);}
}
