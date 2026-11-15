#include "Internal.h"
#include <set>
namespace RetroMediaLab::MediaFileSystems::CommodoreDos::Internal {
FileResult file(const Image& image,std::int32_t track,std::int32_t number,std::vector<Diagnostic>& warnings,const std::string& name) {
    using namespace Constants;checkCancellation(image.token);FileResult result;
    if(!track)return result;
    result.firstBlock=logicalBlock(image.source,track,number);const auto warningStart=warnings.size();std::set<std::pair<std::int32_t,std::int32_t>> seen;
    while(track) {
        checkCancellation(image.token);
        const auto context=std::vector<std::string>{name,std::to_string(track),std::to_string(number)};
        if(!seen.emplace(track,number).second){warnings.push_back({DiagnosticCode::CbmFileCycle,context});result.valid=false;break;}
        result.visited.emplace_back(track,number);
        const auto read=sector(image,track,number);
        if(read.status!=SectorStatus::Success) {
            const auto code=read.status==SectorStatus::InvalidCoordinate?DiagnosticCode::CbmFileCoordinate:read.status==SectorStatus::Truncated?DiagnosticCode::CbmFileTruncated:DiagnosticCode::CbmFileMissing;
            warnings.push_back({code,context});result.valid=false;break;
        }
        const auto& data=read.block->data;const auto nextTrack=data[0],nextSector=data[1];auto used=DataBytes;
        if(!nextTrack) {
            if(nextSector<1||nextSector>DataBytes+1){warnings.push_back({DiagnosticCode::CbmFileFinalCount,{name,std::to_string(nextSector)}});result.valid=false;break;}
            used=nextSector-1;
        }
        result.content.insert(result.content.end(),data.begin()+LinkLength,data.begin()+LinkLength+used);track=nextTrack;number=nextSector;
        if(result.content.size()>image.source.capacity()){warnings.push_back({DiagnosticCode::CbmCapacity,{name}});result.valid=false;break;}
    }
    result.warnings.assign(warnings.begin()+warningStart,warnings.end());return result;
}
}
