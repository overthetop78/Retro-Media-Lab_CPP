#include "Metadata.h"
#include "Constants.h"
#include <QString>
#include <charconv>
namespace RetroMediaLab::MediaEngine::Tzx {
using namespace Constants;
std::string blockIdText(std::uint8_t id){return QString::number(id,HexRadix).rightJustified(HexWidth,QLatin1Char(DigitZero)).toStdString();}
bool tryBlockId(const SequentialSegmentFields& value,std::uint8_t& id){
    const auto found=value.metadata.find(std::string(BlockIdMetadataKey));if(found==value.metadata.end())return false;
    const auto text=QString::fromStdString(found->second).trimmed().toStdString();unsigned parsed{};const auto result=std::from_chars(text.data(),text.data()+text.size(),parsed,HexRadix);
    if(result.ec!=std::errc{}||result.ptr!=text.data()+text.size()||parsed>MaximumByte)return false;id=static_cast<std::uint8_t>(parsed);return true;
}
}
