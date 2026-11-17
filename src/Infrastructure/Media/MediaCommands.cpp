#include "MediaCommands.h"
#include "MediaCommandConstants.h"
#include "Maintenance/GwToolsConstants.h"
#include <QRegularExpression>
#include <QSet>
#include <algorithm>
#include <limits>
#include <stdexcept>
namespace RetroMediaLab::Infrastructure {
namespace C = MediaCommandConstants;
namespace G = GwToolsConstants;
namespace {
[[noreturn]] void invalid() { throw std::invalid_argument(C::InvalidOptions); }
QStringList elements(const QString& text) { const auto result=text.split(G::Colon);if(result.isEmpty() || result.contains(QString{}))invalid();return result; }
QPair<QString,QString> assignment(const QString& text) {const auto parts=text.split(G::Equals);if(parts.size()!=G::AssignmentParts || parts[0].isEmpty() || parts[1].isEmpty())invalid();return {parts[0],parts[1]};}
void unique(QSet<QString>& names,const QString& name) {if(names.contains(name))invalid();names.insert(name);}
bool integer(const QString& value, bool positive) { bool ok=false;const auto number=value.toInt(&ok);return ok && number >= (positive?1:0) && QRegularExpression(C::IntegerPattern).match(value.trimmed()).hasMatch(); }
}
void validateGwTrackSpec(const QString& text) {
    QSet<QString> names;
    for(const auto& element:elements(text)) {
        if(element==G::HeadSwap) {unique(names,element);continue;}
        const auto [key,value]=assignment(element);unique(names,key);
        if(key==G::CylinderKey || key==G::HeadKey) {
            const QRegularExpression pattern(key==G::CylinderKey?G::CylinderPattern:G::HeadPattern);
            for(const auto& range:value.split(G::Comma)) {
                const auto match=pattern.match(range);if(!match.hasMatch())invalid();
                bool ok=false;const auto first=match.captured(1).toInt(&ok);if(!ok)invalid();
                if(!match.captured(G::LastRangeGroup).isEmpty()) {const auto last=match.captured(G::LastRangeGroup).toInt(&ok);if(!ok || first>last)invalid();}
            }
        } else if(key==G::StepKey) {if(!QRegularExpression(G::StepPattern).match(value).hasMatch())invalid();}
        else if(QRegularExpression(G::OffsetKeyPattern).match(key).hasMatch()) {if(!QRegularExpression(G::OffsetPattern).match(value).hasMatch())invalid();}
        else invalid();
    }
    if(!names.contains(G::CylinderKey) || !names.contains(G::HeadKey))invalid();
}
void validateGwPllSpec(const QString& text) {
    QSet<QString> names;
    for(const auto& element:elements(text)) {
        const auto [key,value]=assignment(element);unique(names,key);bool ok=false;
        if(key==G::PeriodKey || key==G::PhaseKey) {value.toInt(&ok);if(!ok)invalid();}
        else if(key==G::LowpassKey) {const auto number=value.toDouble(&ok);if(!ok || !qIsFinite(number) || number<=0)invalid();}
        else invalid();
    }
}
void validateGwOptions(std::span<const MediaEngine::EnabledOption> options) {
    QSet<QString> names;for(const auto& option:options)names.insert(QString::fromStdString(option.argument));
    if((names.contains(C::FakeIndex) && names.contains(C::HardSectors)) || (names.contains(C::Densel) && names.contains(C::Tg43)))invalid();
    for(const auto& option:options) {
        const auto key=QString::fromStdString(option.argument),value=QString::fromStdString(option.value.value_or(std::string{}));
        if(!key.startsWith(C::Prefix) || key.contains(QChar::Null) || value.contains(QChar::Null))invalid();
        if(std::find(C::Flags.begin(),C::Flags.end(),option.argument)!=C::Flags.end()) {if(!value.trimmed().isEmpty())invalid();continue;}
        if(value.trimmed().isEmpty())invalid();
        if((key==C::Retries || key==C::SeekRetries) && !integer(value,false))invalid();
        if(key==C::Revs && !integer(value,true))invalid();
        if(key==C::Densel && value!=C::High && value!=C::Low)invalid();
        if(key==C::Tracks || key==C::OutputTracks)validateGwTrackSpec(value);
        if(key==C::Pll)validateGwPllSpec(value);
        if(key==C::FakeIndex || key==C::AdjustSpeed) {const auto match=QRegularExpression(G::SpeedPattern).match(value);bool ok=false;const auto number=match.captured(1).toDouble(&ok);if(!match.hasMatch() || !ok || !qIsFinite(number) || number<=0)invalid();}
        if(key==C::Precomp) {
            QSet<QString> keys;int thresholds=0;
            for(const auto& part:elements(value)) {
                const auto [name,setting]=assignment(part);unique(keys,name);
                if(name==C::Type) {if(setting!=C::Mfm && setting!=C::Fm && setting!=C::Gcr)invalid();}
                else {bool nameOk=false,valueOk=false;name.toInt(&nameOk);setting.toInt(&valueOk);if(!nameOk || !valueOk || !QRegularExpression(C::ThresholdPattern).match(name).hasMatch())invalid();++thresholds;}
            }
            if(!thresholds)invalid();
        }
    }
}
QStringList tokenizeExpertArguments(const QString& text) {
    QStringList result;QString current;bool quoted=false;
    for(const auto character:text) {
        if(character==QLatin1Char(C::Quote)) {quoted=!quoted;continue;}
        if(character.isSpace() && !quoted) {if(!current.isEmpty()) {result.push_back(current);current.clear();}continue;}
        if(character==QChar::Null)invalid();current+=character;
    }
    if(quoted)invalid();if(!current.isEmpty())result.push_back(current);return result;
}
QString captureSequence(qint64 value,bool alphabetic,int width) {
    if(value<0 || width<1 || width>C::MaximumSequenceWidth)invalid();
    if(!alphabetic)return QString::number(value).rightJustified(width,QLatin1Char(C::DigitZero));
    QString result;
    do {result.prepend(QChar(static_cast<char16_t>(C::LetterA+value%C::AlphabetLength)));value=value/C::AlphabetLength-1;}while(value>=0);
    return result.rightJustified(width,QLatin1Char(C::LetterA));
}
bool parseCaptureSequence(const QString& text,bool alphabetic,qint64& result) {
    if(text.trimmed().isEmpty())return false;
    if(!alphabetic) {bool ok=false;result=text.toLongLong(&ok);return ok && result>=0;}
    result=0;
    for(const auto character:text.trimmed().toUpper()) {
        if(character<QLatin1Char(C::LetterA) || character>QLatin1Char(C::LetterZ))return false;
        const auto digit=character.unicode()-C::LetterA+1;
        if(result>(std::numeric_limits<qint64>::max()-digit)/C::AlphabetLength)return false;
        result=result*C::AlphabetLength+digit;
    }
    --result;return result>=0;
}
}
