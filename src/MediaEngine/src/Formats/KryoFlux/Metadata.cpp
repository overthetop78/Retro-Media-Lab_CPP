#include "Internal.h"
#include <QString>
#include <QLocale>
namespace RetroMediaLab::MediaEngine::KryoFlux {
bool containsInfo(std::span<const std::uint8_t> data){
    using namespace Constants;
    for(std::size_t offset=0;offset<data.size();){
        if(data[offset]==OutOfBand){
            if(data.size()-offset<OutOfBandHeader)return false;
            const auto size=u16(data,offset+SizeOffset);if(size>data.size()-offset-OutOfBandHeader)return false;
            if(data[offset+TypeOffset]==Info)return true;offset+=OutOfBandHeader+size;
        }else offset+=opcodeBytes(data[offset]);
    }
    return false;
}
bool trailingFill(std::span<const std::uint8_t> data,std::size_t offset,std::stop_token token){
    for(;offset<data.size();++offset){checkCancellation(token);if(data[offset]!=Constants::OutOfBand)return false;}return true;
}
void readInfo(std::span<const std::uint8_t> bytes,Metadata& metadata,double& sampleClock){
    using namespace Constants;auto text=QString::fromUtf8(reinterpret_cast<const char*>(bytes.data()),qsizetype(bytes.size()));while(text.endsWith(QChar(Null)))text.chop(1);
    for(const auto& part:text.split(QChar(EntrySeparator),Qt::SkipEmptyParts)){
        const auto line=part.trimmed();const auto separator=line.indexOf(QChar(ValueSeparator));if(separator<=0)continue;
        const auto key=line.first(separator).trimmed(),value=line.mid(separator+1).trimmed();metadata.try_emplace(key.toUtf8().toStdString(),value.toUtf8().toStdString());
        if(key.compare(QString::fromUtf8(SampleClockKey),Qt::CaseInsensitive)==0){bool valid=false;auto locale=QLocale::c();locale.setNumberOptions(QLocale::RejectGroupSeparator);const auto clock=locale.toDouble(value,&valid);if(valid)sampleClock=clock;}
    }
}
void readOutOfBand(std::span<const std::uint8_t> data,std::vector<std::uint32_t>& indexes,Metadata& metadata,double& sampleClock,std::stop_token token){
    using namespace Constants;
    for(std::size_t offset=0;offset<data.size();){checkCancellation(token);
        if(data[offset]==OutOfBand){
            if(trailingFill(data,offset,token))break;
            require(data,offset,OutOfBandHeader);const auto type=data[offset+TypeOffset];const auto size=u16(data,offset+SizeOffset);require(data,offset+OutOfBandHeader,size);const auto payload=data.subspan(offset+OutOfBandHeader,size);
            if(type==Index){if(size<IndexBytes)throw MediaError(DiagnosticCode::KryoIndexTruncated);indexes.push_back(u32(payload,0));}
            else if(type==Info)readInfo(payload,metadata,sampleClock);
            else if(type==EndOfFile)break;
            offset+=OutOfBandHeader+size;
        }else{const auto size=opcodeBytes(data[offset]);require(data,offset,size);offset+=size;}
    }
}
}
