#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::Lisa::Internal {
std::optional<Tag> tag(const SectorBlock& block){
    if(!block.tag||block.tag->size()<Constants::TagLength)return {};
    return Tag{Binary::big<std::uint16_t>(*block.tag,Constants::TagFileIdHigh),std::int32_t(Binary::big<std::uint16_t>(*block.tag,Constants::TagPageHigh)&Constants::PageNumberMask)};
}
bool userFile(std::uint16_t value) noexcept{return value>=Constants::FirstUserFileId&&value<=Constants::LastUserFileId&&std::find(Constants::ReservedFileIds.begin(),Constants::ReservedFileIds.end(),value)==Constants::ReservedFileIds.end();}
std::vector<Page> pages(const SectorImage& image,std::stop_token token){
    std::vector<Page> result;image.visitAvailableBlocks([&](auto number){checkCancellation(token);auto block=image.block(number,token);if(block)result.push_back({block,tag(*block)});return true;},token);return result;
}
}
