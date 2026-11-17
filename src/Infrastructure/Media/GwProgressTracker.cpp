#include "GwProgressTracker.h"
#include "MediaCommandConstants.h"
#include <QRegularExpression>
#include <set>
#include <algorithm>
namespace RetroMediaLab::Infrastructure {
namespace {
namespace C=MediaCommandConstants;
std::vector<int> parseSet(const QString& specification){
    static const QRegularExpression range(QString::fromLatin1(C::ProgressRange));std::set<int> values;
    for(const auto& raw:specification.split(QLatin1Char(C::ProgressComma),Qt::SkipEmptyParts)){const auto match=range.match(raw.trimmed());if(!match.hasMatch())return {};bool valid=false;const auto first=match.captured(C::ProgressFirst).toLongLong(&valid);if(!valid)return {};const auto last=match.captured(C::ProgressSecond).isEmpty()?first:match.captured(C::ProgressSecond).toLongLong(&valid);if(!valid)return {};const auto step=match.captured(C::ProgressThird).isEmpty()?1:match.captured(C::ProgressThird).toLongLong(&valid);if(!valid || step<=0 || last<first || last>=C::MaximumProgressCoordinates)return {};for(auto value=first;value<=last;value+=step)values.insert(static_cast<int>(value));}
    return {values.begin(),values.end()};
}
bool complete(GwTrackState value){return value==GwTrackState::Success || value==GwTrackState::Failed;}
}
void GwProgressTracker::reset(){progress_={};completed_.clear();pending_.clear();discarding_=false;}
void GwProgressTracker::acceptChunk(const QString& chunk,bool finished){
    for(const auto character:chunk){if(character==QLatin1Char(C::ProgressNewline) || character==QLatin1Char(C::ProgressReturn)){if(!discarding_)acceptLine(pending_);pending_.clear();discarding_=false;}else if(!discarding_){pending_+=character;if(pending_.size()>C::MaximumProgressLine){pending_.clear();discarding_=true;}}}
    if(finished){if(!discarding_ && !pending_.isEmpty())acceptLine(pending_);pending_.clear();discarding_=false;}
}
void GwProgressTracker::acceptLine(const QString& line){
    static const QRegularExpression header(QString::fromLatin1(C::ProgressHeader),QRegularExpression::CaseInsensitiveOption),track(QString::fromLatin1(C::ProgressTrack),QRegularExpression::CaseInsensitiveOption);
    const auto configuration=header.match(line);if(configuration.hasMatch()){progress_.cylinders=parseSet(configuration.captured(C::ProgressFirst));progress_.heads=parseSet(configuration.captured(C::ProgressSecond));progress_.total.reset();progress_.totalOnHead.reset();if(!progress_.cylinders.empty()){progress_.totalOnHead=static_cast<int>(progress_.cylinders.size());if(!progress_.heads.empty() && progress_.heads.size()<=C::MaximumProgressCoordinates/progress_.cylinders.size())progress_.total=static_cast<int>(progress_.cylinders.size()*progress_.heads.size());}}
    const auto match=track.match(line);if(!match.hasMatch())return;bool valid=false;const auto cylinder=match.captured(C::ProgressFirst).toInt(&valid);if(!valid)return;const auto head=match.captured(C::ProgressSecond).toInt();auto state=GwTrackState::Success;if(line.contains(QString::fromLatin1(C::RetryToken),Qt::CaseInsensitive))state=GwTrackState::Retry;else if(line.contains(QString::fromLatin1(C::FailedToken),Qt::CaseInsensitive) || line.contains(QString::fromLatin1(C::ErrorToken),Qt::CaseInsensitive))state=GwTrackState::Failed;
    progress_.cylinder=cylinder;progress_.head=head;progress_.states[{cylinder,head}]=state;if(complete(state))completed_.insert({cylinder,head});progress_.completedPerHead.clear();for(const auto& address:completed_)++progress_.completedPerHead[address.second];progress_.completed=static_cast<int>(completed_.size());progress_.completedOnHead=static_cast<int>(std::count_if(completed_.begin(),completed_.end(),[head](const auto& item){return item.second==head;}));
    progress_.nextCylinder.reset();progress_.nextHead.reset();for(const auto c:progress_.cylinders){for(const auto h:progress_.heads){if(!completed_.contains({c,h})){progress_.nextCylinder=c;progress_.nextHead=h;break;}}if(progress_.nextCylinder)break;}
}
}
