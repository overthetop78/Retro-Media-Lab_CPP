#include "LocalizedText.h"
#include "Constants.h"
namespace RetroMediaLab::MediaEngine::Decoding {
LocalizedText literal(std::string value){return {{},std::move(value),{}};}
LocalizedText message(std::string_view key,std::vector<LocalizedText> arguments){return {std::string(key),{},std::move(arguments)};}
std::string resolveText(const LocalizedText& text,const TextResolver& resolver){
 if(text.key.empty())return text.literal;const auto pattern=resolver(text.key);std::vector<std::string> arguments;for(const auto& a:text.arguments)arguments.push_back(resolveText(a,resolver));std::string result;
 for(std::size_t i=0;i<pattern.size();){
  if(pattern[i]==Constants::PlaceholderStart){auto end=i+1;std::size_t index=0;bool valid=false;
   while(end<pattern.size()&&pattern[end]>=Constants::DigitFirst&&pattern[end]<=Constants::DigitLast){const auto digit=std::size_t(pattern[end]-Constants::DigitFirst);if(index>arguments.size()/Constants::DecimalBase){valid=false;break;}index=index*Constants::DecimalBase+digit;valid=true;++end;}
   if(valid&&end<pattern.size()&&pattern[end]==Constants::PlaceholderEnd&&index<arguments.size()){result+=arguments[index];i=end+1;continue;}
  }
  result+=pattern[i++];
 }
 return result;
}
}
