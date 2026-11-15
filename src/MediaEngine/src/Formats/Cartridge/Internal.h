#pragma once
#include "Readers.h"
#include "Constants.h"
#include "../FormatIds.h"
#include "../../Models/Blocks/BlockImage.h"
#include "../../Sources/AtomicImageWriter.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <RetroMediaLab/MediaFileSystems/MetadataNumbers.h>
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaEngine::Cartridge {
namespace M=MediaFileSystems;
namespace Internal {
using namespace Constants;
inline bool starts(M::Bytes bytes,std::string_view text){return bytes.size()>=text.size()&&std::equal(text.begin(),text.end(),bytes.begin(),[](char a,std::uint8_t b){return std::uint8_t(a)==b;});}
inline bool fdsSignature(M::Bytes bytes){return bytes.size()>=FdsSignature.size()&&std::equal(FdsSignature.begin(),FdsSignature.end(),bytes.begin());}
template<class T> T little(M::Bytes bytes,std::size_t offset){T value=0;for(std::size_t index=0;index<sizeof(T);++index)value|=T(T(bytes[offset+index])<<(index*std::numeric_limits<std::uint8_t>::digits));return value;}
template<class T> void little(M::MutableBytes bytes,std::size_t offset,T value){for(std::size_t index=0;index<sizeof(T);++index)bytes[offset+index]=std::uint8_t(value>>(index*std::numeric_limits<std::uint8_t>::digits));}
inline std::vector<std::uint8_t> readSource(const std::shared_ptr<const M::RandomAccessData>& source,std::uint64_t offset,std::size_t length,std::stop_token token){M::checkCancellation(token);if(offset>source->length()||length>source->length()-offset)throw M::MediaError(M::DiagnosticCode::SourceEnded,{std::to_string(std::max(offset,source->length()))});std::vector<std::uint8_t> bytes(length);source->readExactly(offset,bytes,token);return bytes;}
inline std::string padded(std::int32_t value){auto text=std::to_string(value);if(text.size()<std::size_t(NameDigits))text.insert(text.begin(),std::size_t(NameDigits)-text.size(),Zero);return text;}
inline std::string bankKey(std::int32_t bank,std::string_view suffix){return std::string(BankPrefix)+std::to_string(bank)+std::string(suffix);}
inline M::MessageRecipe message(DiagnosticCode code,std::vector<std::string> arguments={}){return {std::string(diagnosticKey(code)),std::move(arguments),std::string(Domain)};}
inline M::MessageRecipe label(std::string_view key){return {std::string(key),{},std::string(Domain)};}
inline M::VolumeDescriptor volume(std::int64_t capacity,std::string_view key){return {0,capacity,std::string(M::VolumeConstants::DirectVolume),{},{},{},{},{},{},{},{},label(key)};}
inline bool requested(const RecognitionContext& context,std::string_view id){const auto value=context.requestedFormatId();return !value||sameIdentifier(*value,id);}
inline std::filesystem::path sourcePath(std::string_view value){std::u8string utf8;utf8.reserve(value.size());for(const auto byte:value)utf8.push_back(char8_t(byte));return std::filesystem::path(utf8);}
inline std::int32_t integer(std::int64_t value){if(value<0||value>std::numeric_limits<std::int32_t>::max())throw M::MediaError(M::DiagnosticCode::BlockOverflow);return std::int32_t(value);}
}
}
