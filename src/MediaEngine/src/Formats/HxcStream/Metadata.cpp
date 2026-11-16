#include "Internal.h"
#include <QString>
namespace RetroMediaLab::MediaEngine::HxcStream {
void readMetadata(std::span<const std::uint8_t> bytes,Metadata& metadata,std::int32_t& resolution) {
    using namespace Constants;auto text=QString::fromUtf8(reinterpret_cast<const char*>(bytes.data()),qsizetype(bytes.size()));while(text.endsWith(QChar(Null)))text.chop(1);
    std::size_t begin{};const auto content=text.toUtf8().toStdString();
    while(begin<content.size()){const auto end=content.find_first_of(LineSeparators,begin);const auto piece=std::string_view(content).substr(begin,end==std::string::npos?content.size()-begin:end-begin);const auto line=QString::fromUtf8(piece.data(),qsizetype(piece.size())).trimmed();
        const auto separator=line.indexOf(QChar(Space));if(separator>0){const auto key=line.first(separator),value=line.mid(separator+1).trimmed();metadata.try_emplace(key.toUtf8().toStdString(),value.toUtf8().toStdString());
            if(key==QString::fromUtf8(SampleRateKey.data(),qsizetype(SampleRateKey.size()))){bool valid=false;const auto rate=value.toInt(&valid);if(valid&&(rate==LowSampleRate||rate==HighSampleRate))resolution=std::int32_t(NanosecondsPerSecond/std::uint32_t(rate));}
        }
        if(end==std::string::npos)break;begin=end+1;
    }
}
}
