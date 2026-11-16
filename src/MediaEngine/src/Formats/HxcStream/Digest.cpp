#include "Stream.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QCryptographicHash>
#include <QByteArrayView>
#include <array>
namespace RetroMediaLab::MediaEngine::HxcStream {
namespace {
std::string key(std::string_view prefix,std::int32_t cylinder,std::int32_t head){return std::string(prefix)+std::to_string(cylinder)+Constants::Separator+std::to_string(head);}
}
std::string nativeKey(std::int32_t cylinder,std::int32_t head){return key(Constants::NativePrefix,cylinder,head);}
std::string digestKey(std::int32_t cylinder,std::int32_t head){return key(Constants::DigestPrefix,cylinder,head);}
std::string fieldsPrefix(std::int32_t cylinder,std::int32_t head){return key(Constants::FieldsPrefix,cylinder,head)+Constants::Separator;}
std::string digest(const Flux::ProtectedTrack& track,bool writeProtected) {
    using namespace Constants;QCryptographicHash hash(QCryptographicHash::Sha256);
    const auto integer=[&](std::uint32_t value){std::array<char,FieldBytes> bytes{};for(std::size_t i=0;i<FieldBytes;++i)bytes[i]=char(value>>(i*ByteBits));hash.addData(QByteArrayView(bytes.data(),bytes.size()));};
    integer(std::uint32_t(track.cylinder()));integer(std::uint32_t(track.head()));const char flag=writeProtected?TrueByte:FalseByte;hash.addData(QByteArrayView(&flag,1));
    for(const auto& revolution:track.revolutions()){
        integer(std::uint32_t(revolution.resolutionNanoseconds()));integer(revolution.flux().indexTimeTicks);
        if(revolution.flux().intervals.size()>MaximumSignedSize)throw MediaError(DiagnosticCode::HxcChunkLength);
        integer(std::uint32_t(revolution.flux().intervals.size()));for(const auto interval:revolution.flux().intervals)integer(interval);
    }return hash.result().toHex().toUpper().toStdString();
}
}
