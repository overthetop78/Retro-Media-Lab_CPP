#include "DiskDefinitions.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QString>
#include <QStringList>
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaEngine::Definitions {
namespace {
QString text(std::string_view value){return QString::fromUtf8(value.data(),qsizetype(value.size()));}
std::string value(const TrackDefinition& track,std::string_view key,std::string fallback={}){const auto found=track.parameters.find(key);return found==track.parameters.end()?std::move(fallback):found->second;}
std::vector<std::string> split(std::string_view input,char separator){std::vector<std::string> result;for(const auto& part:text(input).split(QChar::fromLatin1(separator)))result.push_back(part.toUtf8().toStdString());return result;}
[[noreturn]] void invalid(std::string_view field,std::string_view value){throw MediaError(DiagnosticCode::DefinitionValue,{std::string(field),std::string(value)});}
}
bool IdentifierLess::operator()(std::string_view a,std::string_view b)const{return QString::compare(text(a),text(b),Qt::CaseInsensitive)<0;}
std::int32_t parseNumber(std::string_view field,std::string_view input){bool valid=false;const auto number=text(input).toInt(&valid,Constants::DecimalBase);if(!valid)invalid(field,input);return number;}
std::int32_t TrackDefinition::number(std::string_view key,std::int32_t fallback)const{const auto found=parameters.find(key);return found==parameters.end()?fallback:parseNumber(key,found->second);}
bool TrackDefinition::matches(std::int32_t cylinder,std::int32_t head)const{
    if(selection==Constants::Wildcard)return true;
    for(const auto& range:split(selection,Constants::ListSeparator)){
        const auto parts=split(range,Constants::HeadSeparator);
        if(parts.size()>Constants::HeadPart&&parseNumber(selection,parts[Constants::HeadPart])!=head)continue;
        const auto bounds=split(parts.front(),Constants::RangeSeparator);const auto first=parseNumber(selection,bounds.front());
        const auto last=bounds.size()>Constants::RangeLastPart?parseNumber(selection,bounds[Constants::RangeLastPart]):first;
        if(cylinder>=first&&cylinder<=last)return true;
    }
    return false;
}
const TrackDefinition* DiskDefinition::track(std::int32_t cylinder,std::int32_t head)const{
    for(auto it=tracks.rbegin();it!=tracks.rend();++it)if(it->selection!=Constants::Wildcard&&it->matches(cylinder,head))return &*it;
    const auto found=std::find_if(tracks.begin(),tracks.end(),[](const auto& item){return item.selection==Constants::Wildcard;});
    return found==tracks.end()?nullptr:&*found;
}
std::vector<Sector> DiskDefinition::layout()const{
    std::vector<Sector> result;
    for(std::int32_t cylinder=0;cylinder<cylinders;++cylinder)for(std::int32_t head=0;head<heads;++head){
        const auto* selected=track(cylinder,head);if(!selected)continue;
        const auto& t=*selected;
        auto defaultSize=Constants::DefaultSector;
        if(t.codec==Constants::AppleGcr||t.codec==Constants::Hp)defaultSize=Constants::SmallSector;
        else if(t.codec==Constants::Micropolis)defaultSize=t.number(Constants::ImageBps,Constants::SmallSector);
        else if(t.codec==Constants::Northstar&&value(t,Constants::Mode)==Constants::FmMode)defaultSize=Constants::SmallSector;
        const auto expression=value(t,Constants::Bps,std::to_string(defaultSize));std::vector<std::int32_t> sizes;
        for(const auto& term:split(expression,Constants::ListSeparator)){
            const auto parts=split(term,Constants::RepeatSeparator);const auto size=parseNumber(Constants::Bps,parts.front());
            const auto repeat=parts.size()>Constants::RepeatCountPart?parseNumber(Constants::Bps,parts[Constants::RepeatCountPart]):1;
            if(size<=0||repeat<0||std::uint64_t(repeat)>std::uint64_t(std::numeric_limits<std::int32_t>::max())-sizes.size())invalid(Constants::Bps,expression);
            sizes.insert(sizes.end(),std::size_t(repeat),size);
        }
        std::vector<std::int32_t> physical;
        if(t.codec==Constants::AppleGcr){const auto found=t.parameters.find(Constants::Sectors);if(found==t.parameters.end())invalid(Constants::Sectors,{});for(const auto& item:split(found->second,Constants::ListSeparator))physical.push_back(parseNumber(Constants::Sectors,item));}
        const auto count=t.codec==Constants::AppleGcr?std::int32_t(physical.size()):t.number(Constants::Sectors,t.codec==Constants::DataGeneral?Constants::DataGeneralSectors:0);
        const auto first=t.number(Constants::FirstSector,t.codec==Constants::IbmFm||t.codec==Constants::IbmMfm?Constants::FirstIbmSector:0);
        if(count>0&&sizes.empty())invalid(Constants::Bps,expression);
        for(std::int32_t index=0;index<count;++index){
            const auto size=sizes[std::size_t(index)%sizes.size()];const auto number=t.codec==Constants::AppleGcr?std::int64_t(physical[std::size_t(index)]):std::int64_t(first)+index;
            const auto imageSize=t.number(Constants::ImageBps,size);
            if(imageSize<=0||number>std::numeric_limits<std::int32_t>::max())invalid(Constants::ImageBps,std::to_string(imageSize));
            result.push_back({result.size(),cylinder,head,std::int32_t(number),size,imageSize,std::size_t(selected-tracks.data())});
        }
    }
    return result;
}
}
