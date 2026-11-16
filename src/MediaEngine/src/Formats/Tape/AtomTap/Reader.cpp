#include "Reader.h"
#include "Constants.h"
#include "../Uef/Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
namespace RetroMediaLab::MediaEngine::AtomTap {
using namespace Constants;
const ReaderCapabilities& Reader::capabilities()const{static const ReaderCapabilities caps{{std::string(FormatId)},{std::string(Extension)},{},{},{MediaKind::Tape},{RepresentationKind::Sequential}};return caps;}
bool Reader::supportsFormatId(std::string_view id)const{return id==FormatId;}
bool Reader::validate(std::span<const std::uint8_t> bytes,std::stop_token token){if(bytes.empty())return false;std::size_t offset{};while(offset<bytes.size()){checkCancellation(token);if(bytes.size()-offset<FileHeaderBytes)return false;const auto header=bytes.subspan(offset,FileHeaderBytes);bool terminated=false;for(const auto value:header.first(FileNameBytes)){if(!value){terminated=true;continue;}if(terminated||value<FirstPrintable||value>LastPrintable)return false;}if(!header[0])return false;const auto length=qFromLittleEndian<std::uint16_t>(header.data()+LengthOffset);offset+=FileHeaderBytes;if(length>bytes.size()-offset)return false;offset+=length;}return true;}
std::future<bool> Reader::canReadAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{if(context.requestedFormatId()&&!Reader{}.supportsFormatId(*context.requestedFormatId()))return false;if(!sameIdentifier(context.extension(),Extension)||context.length()<FileHeaderBytes||context.length()>Uef::Constants::MaximumDecompressedLength)return false;return validate(context.readBytesAsync(token).get(),token);});}
std::future<std::shared_ptr<const MediaDocument>> Reader::readAsync(RecognitionContext context,std::stop_token token)const{return std::async(std::launch::deferred,[context,token]{if(context.length()>Uef::Constants::MaximumDecompressedLength)throw MediaError(DiagnosticCode::AtomTapLimit);const auto bytes=context.readBytesAsync(token).get();return readBytes(context.source(),bytes,token);});}
}
