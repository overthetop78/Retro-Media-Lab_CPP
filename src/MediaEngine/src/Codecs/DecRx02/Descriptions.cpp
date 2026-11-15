#include "Descriptions.h"
namespace RetroMediaLab::MediaEngine::DecRx02 {
namespace {
using namespace Constants;
Decoding::LocalizedText state(std::optional<bool> valid){return Decoding::message(valid?(*valid?ValidKey:InvalidKey):UnavailableKey);}
std::string hex(std::uint8_t value){return {HexDigits[value>>BitsPerNibble],HexDigits[value&NibbleMask]};}
std::vector<Decoding::LocalizedText> identity(const Header& header,std::int32_t size){using Decoding::literal;return {literal(std::to_string(header.cylinder)),literal(std::to_string(header.head)),literal(std::to_string(header.sector)),literal(std::to_string(size))};}
std::string encodingName(const DataMark& mark){return std::string(mark.encoding==DataEncoding::M2Fm?M2FmName:FmName);}
}
Decoding::LocalizedText headerDescription(const Header& header,const DataMark* mark,std::optional<bool> valid){
 auto args=identity(header,mark?mark->sectorSize:0);if(mark){args.push_back(Decoding::literal(hex(mark->mark)));args.push_back(Decoding::literal(encodingName(*mark)));}args.push_back(state(header.crcValid));args.push_back(state(valid));return Decoding::message(mark?Constants::HeaderKey:Constants::HeaderNoMarkKey,std::move(args));
}
Decoding::LocalizedText dataDescription(const Header& header,const DataMark& mark,bool valid){auto args=identity(header,mark.sectorSize);args.push_back(Decoding::literal(hex(mark.mark)));args.push_back(Decoding::literal(encodingName(mark)));args.push_back(state(valid));return Decoding::message(Constants::DataKey,std::move(args));}
Decoding::LocalizedText unpairedDescription(const DataMark& mark){return Decoding::message(Constants::UnpairedKey,{Decoding::literal(hex(mark.mark))});}
}
