#include "Adapter.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QByteArray>
namespace RetroMediaLab::MediaEngine::Jfd {
std::map<std::string,std::string> metadata(const Image& image,bool gzip) {
    using namespace Constants;
    return {{std::string(NativeKey),QByteArray(reinterpret_cast<const char*>(image.bytes.data()),qsizetype(image.bytes.size())).toBase64().toStdString()},{std::string(GzipKey),std::string(gzip?TrueText:FalseText)}};
}
std::optional<Image> restore(const std::map<std::string,std::string>& values,std::stop_token token) {
    using namespace Constants;const auto found=values.find(std::string(NativeKey));if(found==values.end())return {};
    QByteArray encoded;encoded.reserve(qsizetype(found->second.size()));
    for(char value:found->second)if(Base64Whitespace.find(value)==std::string_view::npos)encoded.append(value);
    if(encoded.size()%Base64Quantum!=0)throw MediaError(DiagnosticCode::JfdNativeMetadata);
    const auto decoded=QByteArray::fromBase64Encoding(encoded,QByteArray::AbortOnBase64DecodingErrors);
    if(!decoded)throw MediaError(DiagnosticCode::JfdNativeMetadata);
    return decode({reinterpret_cast<const std::uint8_t*>(decoded.decoded.constData()),std::size_t(decoded.decoded.size())},token);
}
}
