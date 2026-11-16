#include "Adapter.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QByteArray>
#include <QRegularExpression>
#include <QString>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::HxcStream {
namespace {
bool native(const MediaDocument& document,const Flux::ProtectedTrack& track,bool protection){
    const auto& values=document.metadata();const auto found=values.find(digestKey(track.cylinder(),track.head()));
    return values.contains(nativeKey(track.cylinder(),track.head()))&&found!=values.end()&&found->second==digest(track,protection);
}
QString suffix(const Flux::ProtectedTrack& track){return QString::number(track.cylinder()).rightJustified(Constants::CylinderDigits,QChar(Constants::ZeroDigit))+QChar(Constants::Separator)+QString::number(track.head());}
std::vector<std::uint8_t> restore(const std::string& value){
    QByteArray encoded;encoded.reserve(qsizetype(value.size()));
    for(const char byte:value)if(Constants::Base64Whitespace.find(byte)==std::string_view::npos)encoded.append(byte);
    if(encoded.size()%Constants::Base64Quantum!=0)throw MediaError(DiagnosticCode::HxcNativeMetadata);
    const auto decoded=QByteArray::fromBase64Encoding(encoded,QByteArray::AbortOnBase64DecodingErrors);
    if(!decoded)throw MediaError(DiagnosticCode::HxcNativeMetadata);
    const auto* begin=reinterpret_cast<const std::uint8_t*>(decoded.decoded.constData());return {begin,begin+decoded.decoded.size()};
}
}
std::shared_ptr<const ImageWriter> writerAdapter(Publisher publish){
    using namespace Constants;
    if(!publish)publish=[](auto files,auto token){writeAtomicImageSet(files,token);};
    TypedWriterPredicate<Flux::ProtectedTrackImage> predicate=[](const auto& image,const MediaDocument& document,auto,auto){
        return document.mediaKind()==MediaKind::Floppy&&!image.tracks().empty()&&std::all_of(image.tracks().begin(),image.tracks().end(),[&](const auto& track){
            return track.cylinder()<=std::int32_t(MaximumTrack)&&track.head()<=std::int32_t(MaximumHead)&&std::none_of(track.features().begin(),track.features().end(),[](const auto& feature){return feature.kind==Flux::TrackFeatureKind::WeakRegion;})&&(native(document,track,image.writeProtected())||canEncode(track));
        });
    };
    TypedWriterOutput<Flux::ProtectedTrackImage> writer=[publish=std::move(publish)](const auto& image,const MediaDocument& document,const std::filesystem::path& output,auto,std::stop_token token){
        checkCancellation(token);
        const auto stem=QString::fromStdU16String(output.stem().u16string());const auto firstSuffix=suffix(image.tracks().front());
        if(!QRegularExpression(QString::fromUtf8(OutputStemPattern)).match(stem).hasMatch()||!stem.endsWith(firstSuffix))throw MediaError(DiagnosticCode::HxcOutputName,{(firstSuffix+QString::fromUtf8(Extension)).toStdString()});
        const auto prefix=stem.left(stem.size()-firstSuffix.size());
        std::vector<ImageFileOutput> files;std::vector<std::filesystem::path> paths;files.reserve(image.tracks().size());paths.reserve(image.tracks().size());
        for(const auto& track:image.tracks()){
            checkCancellation(token);
            auto bytes=native(document,track,image.writeProtected())?restore(document.metadata().at(nativeKey(track.cylinder(),track.head()))):encode(track,image.writeProtected(),document.metadata(),token);
            auto path=output.parent_path()/std::filesystem::path((prefix+suffix(track)+QString::fromUtf8(Extension)).toStdU16String());paths.push_back(path);
            files.push_back({std::move(path),[bytes=std::move(bytes)](const ImageOutput& write,std::stop_token stop){checkCancellation(stop);write(bytes);}});
        }
        publish(files,token);return paths;
    };
    return fluxWriterAdapter(std::string(WriterId),{std::string(FormatId)},{std::string(Extension)},std::move(predicate),std::move(writer),true);
}
}
