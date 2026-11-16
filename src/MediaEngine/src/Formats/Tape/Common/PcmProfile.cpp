#include "Pcm.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QString>
#include <algorithm>
#include <limits>
namespace RetroMediaLab::MediaEngine::TapePcm {
namespace {bool number(const Parameters& values,std::string_view key,int& output){const auto found=values.find(std::string(key));if(found==values.end())return false;bool valid=false;output=QString::fromUtf8(found->second).toInt(&valid);return valid;}bool flag(const std::string& value){const auto text=QString::fromUtf8(value).trimmed();if(text.compare(QString::fromUtf8(Constants::TrueValue),Qt::CaseInsensitive)==0)return true;if(text.compare(QString::fromUtf8(Constants::FalseValue),Qt::CaseInsensitive)==0)return false;throw MediaError(DiagnosticCode::TapePcmProfile);}}
void validate(const Profile& value){using namespace Constants;if(value.sampleRate<MinimumRate||value.sampleRate>MaximumRate||value.channels<1||value.channels>MaximumRawChannels||std::find(SupportedBits.begin(),SupportedBits.end(),value.bits)==SupportedBits.end())throw MediaError(DiagnosticCode::TapePcmProfile);}
Profile readProfile(const Parameters* values){using namespace Constants;Profile result;if(values){for(const auto& [key,target]:std::array<std::pair<std::string_view,int*>,ProfileNumberFields>{{{RateKey,&result.sampleRate},{ChannelsKey,&result.channels},{BitsKey,&result.bits}}})if(values->contains(std::string(key))&&!number(*values,key,*target))throw MediaError(DiagnosticCode::TapePcmProfile);if(const auto entry=values->find(std::string(SignedKey));entry!=values->end())result.signedSamples=flag(entry->second);if(const auto entry=values->find(std::string(EndianKey));entry!=values->end())result.littleEndian=flag(entry->second);}validate(result);return result;}
bool tryProfile(const Parameters& values,int& channels,int& rate,int& bits,int& align){using namespace Constants;channels=rate=bits=align=0;return number(values,ChannelsKey,channels)&&number(values,RateKey,rate)&&number(values,BitsKey,bits)&&number(values,AlignKey,align)&&channels>0&&rate>0&&std::find(SupportedBits.begin(),SupportedBits.end(),bits)!=SupportedBits.end()&&std::int64_t(channels)*(bits/ByteBits)==align;}
bool trySampleRate(const Parameters& values,int& rate){return number(values,Constants::RateKey,rate)&&rate>=Constants::MinimumRate;}
}
