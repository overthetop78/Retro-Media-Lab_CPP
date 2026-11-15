#include "Internal.h"
namespace RetroMediaLab::MediaFileSystems::ProDos::Internal {
using namespace Constants;
Bitmap bitmap(const SectorImage& image,std::int32_t start,std::int32_t total,std::vector<Diagnostic>& warnings,std::stop_token token){
    Bitmap result;
    for(std::int32_t number=0;number<total;++number){
        checkCancellation(token);const auto map=start+number/std::int32_t(BlocksPerBitmapBlock);const auto block=image.block(std::uint64_t(map),token);
        if(!block||block->data.size()!=BlockSize){if(number%BlocksPerBitmapBlock==0)warnings.push_back({DiagnosticCode::ProDosBitmap,{std::to_string(map)}});result.valid=false;continue;}
        const auto bit=std::uint32_t(number)%BlocksPerBitmapBlock;
        if(block->data[bit/BitsPerByte]&(BitmapHighBitMask>>(bit&(BitsPerByte-1))))++result.freeBlocks;
    }return result;
}
}
