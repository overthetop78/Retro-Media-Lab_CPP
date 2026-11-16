#include "Sheet.h"
#include "../../../Primitives/Identifiers.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QStringDecoder>
namespace RetroMediaLab::MediaEngine::BinCue {
namespace {
using namespace Constants;
QString text(std::string_view value){QStringDecoder decoder(QStringConverter::Utf8,QStringConverter::Flag::ConvertInitialBom|QStringConverter::Flag::Stateless);const QString result=decoder(QByteArrayView(value.data(),qsizetype(value.size())));if(decoder.hasError())throw MediaError(DiagnosticCode::CueOutput);return result;}
QString padded(std::int64_t value){return QString::number(qlonglong(value)).rightJustified(TimeWidth,QChar(Zero));}
QString quoteText(std::string_view value){auto result=text(value);result.replace(QString(1,QChar(Quote)),QString(TokenPair,QChar(Quote)));return QString(1,QChar(Quote))+result+QChar(Quote);}
QString time(std::int64_t value){if(value<0)throw MediaError(DiagnosticCode::CueOutput);const auto minutes=value/(SecondsPerMinute*FramesPerSecond),remainder=value%(SecondsPerMinute*FramesPerSecond);return padded(minutes)+QChar(Colon)+padded(remainder/FramesPerSecond)+QChar(Colon)+padded(remainder%FramesPerSecond);}
QString mode(OpticalTrackMode value){for(const auto& item:Modes)if(item.mode==value)return text(item.name);throw MediaError(DiagnosticCode::CueOutput);}
void line(QString& output,QString value){while(!value.isEmpty()&&value.back().isSpace())value.chop(1);output+=value+text(NewLine);}
void metadata(QString& output,const OpticalMetadata& values,const QString& indent,std::stop_token token){
    for(const auto& [key,value]:values){checkCancellation(token);QString command;if(Primitives::ordinalIdentifierStartsWithIgnoreCase(key,RemPrefix))command=text(Rem)+QChar(Space)+text(std::string_view(key).substr(RemPrefix.size()));else command=text(key).toUpper();
        if(command!=text(Title)&&command!=text(Performer)&&command!=text(Songwriter)&&!command.startsWith(text(Rem)+QChar(Space)))throw MediaError(DiagnosticCode::CueOutput);const auto content=key==RemLoop||key==RemNoLoop?text(value):quoteText(value);line(output,indent+command+QChar(Space)+content);}
}
}
std::string serializeSheet(const Sheet& sheet,std::stop_token token){
    using namespace Constants;checkCancellation(token);QString output;if(sheet.catalog())line(output,text(Catalog)+QChar(Space)+text(*sheet.catalog()));metadata(output,sheet.metadata(),{},token);
    for(const auto& file:sheet.files()){checkCancellation(token);std::string_view type;switch(file.kind()){case FileKind::Binary:type=BinaryFile;break;case FileKind::WavePcm:type=WaveFile;break;case FileKind::Vorbis:type=VorbisFile;break;default:throw MediaError(DiagnosticCode::CueOutput);}line(output,text(File)+QChar(Space)+quoteText(file.path())+QChar(Space)+text(type));
        for(const auto& declaration:file.tracks()){checkCancellation(token);const auto& track=declaration.fields();const auto indent=text(Indent),nested=indent+indent;line(output,indent+text(Track)+QChar(Space)+padded(track.number)+QChar(Space)+mode(track.mode));metadata(output,track.metadata,nested,token);
            if(!track.flags.empty()){QString flags=nested+text(Flags);for(const auto& flag:track.flags)flags+=QChar(Space)+text(flag);line(output,std::move(flags));}if(track.isrc)line(output,nested+text(Isrc)+QChar(Space)+text(*track.isrc));if(track.pregap>0)line(output,nested+text(Pregap)+QChar(Space)+time(track.pregap));
            for(const auto& index:track.indexes){if(index.relativeSector()<0)throw MediaError(DiagnosticCode::CueOutput);line(output,nested+text(Index)+QChar(Space)+padded(index.number())+QChar(Space)+time(index.relativeSector()));}if(track.postgap>0)line(output,nested+text(Postgap)+QChar(Space)+time(track.postgap));
        }
    }
    return output.toUtf8().toStdString();
}
}
