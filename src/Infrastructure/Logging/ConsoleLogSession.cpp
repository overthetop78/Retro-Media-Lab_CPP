#include "ConsoleLogSession.h"
#include <ErrorKeys.h>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSaveFile>
#include <deque>
#include <stdexcept>
namespace RetroMediaLab {
ConsoleLogSession::ConsoleLogSession(QString directory,std::function<LoggingSettings()> settings,std::function<QDateTime()> clock)
    :directory_(std::move(directory)),settings_(std::move(settings)),clock_(std::move(clock)) {
    if(!settings_||!clock_)throw std::invalid_argument(ErrorKeys::InvalidLoggingSettings);
}
bool ConsoleLogSession::begin(const QString& action,const QString& command) noexcept {
    try {
        const std::lock_guard lock(gate_);
        action_=action.toLower().replace(QRegularExpression(LogConstants::InvalidNamePattern),LogConstants::Dash);
        if(action_.trimmed().isEmpty())action_=LogConstants::Operation;
        path_=QDir(directory_).filePath(action_+LogConstants::LogExtension);
        return appendLocked(LogConstants::NewLine)
            &&appendLocked(QString(LogConstants::Separator)+LogConstants::NewLine)
            &&appendLocked(clock_().toString(Qt::ISODateWithMs)+LogConstants::NewLine)
            &&appendLocked(QString(LogConstants::CommandPrefix)+command+LogConstants::NewLine);
    }catch(...){return false;}
}
bool ConsoleLogSession::appendLine(const QString& line) noexcept {
    try{return appendText(line+LogConstants::NewLine);}catch(...){return false;}
}
bool ConsoleLogSession::appendText(const QString& text) noexcept {
    try{const std::lock_guard lock(gate_);return appendLocked(text);}catch(...){return false;}
}
bool ConsoleLogSession::appendLocked(const QString& entry) {
    if(path_.isEmpty())return true;
    const auto settings=settings_().forAction(action_);
    if(!settings.enabled)return true;
    if(!QDir().mkpath(directory_))return false;
    const qint64 maximum=qMax(LogConstants::Unlimited,settings.maximumKilobytes)*qint64(LogConstants::BytesPerKiB);
    const auto bytes=entry.toUtf8();
    if(maximum>LogConstants::Unlimited&&QFileInfo::exists(path_)&&QFileInfo(path_).size()+bytes.size()>maximum){
        if(settings.keepArchives){if(!archive())return false;}
        else return trim(entry,maximum);
    }
    QFile file(path_);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Append))return false;
    return file.write(bytes)==bytes.size()&&file.flush();
}
bool ConsoleLogSession::archive() {
    const auto name=action_+LogConstants::Dash+clock_().toString(LogConstants::ArchiveTimestamp);
    auto destination=QDir(directory_).filePath(name+LogConstants::LogExtension);
    for(int suffix=LogConstants::FirstArchiveSuffix;QFileInfo::exists(destination);++suffix)
        destination=QDir(directory_).filePath(name+LogConstants::Dash+QString::number(suffix)+LogConstants::LogExtension);
    return QFile::rename(path_,destination);
}
bool ConsoleLogSession::trim(const QString& entry,qint64 maximum) {
    QFile input(path_);if(!input.open(QIODevice::ReadOnly))return false;
    const auto contents=QString::fromUtf8(input.readAll());if(input.error()!=QFileDevice::NoError)return false;input.close();
    auto lines=contents.split(QRegularExpression(LogConstants::LineBreakPattern));
    if(contents.isEmpty())lines.clear();
    if(contents.endsWith(LogConstants::LineFeed)||contents.endsWith(LogConstants::CarriageReturn))lines.removeLast();
    auto incoming=entry;
    while(incoming.endsWith(LogConstants::LineFeed)||incoming.endsWith(LogConstants::CarriageReturn))incoming.chop(1);
    lines.append(incoming);std::deque<QByteArray> retained;qint64 size=0;
    for(auto it=lines.crbegin();it!=lines.crend();++it){
        const auto line=it->toUtf8()+LogConstants::NewLine;
        if(!retained.empty()&&size+line.size()>maximum)break;
        if(line.size()>maximum&&retained.empty())continue;
        retained.push_front(line);size+=line.size();
    }
    QSaveFile output(path_);if(!output.open(QIODevice::WriteOnly))return false;
    for(const auto& line:retained)if(output.write(line)!=line.size())return false;
    return output.commit();
}
}
