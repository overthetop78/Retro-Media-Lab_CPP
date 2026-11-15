#pragma once
#include "MetadataNumberConstants.h"
#include <charconv>
#include <cstdint>
#include <map>
#include <string>
#include <type_traits>
namespace RetroMediaLab::MediaFileSystems::MetadataNumbers {
enum class IntegerStyle {None,Integer};
template<class T> bool decimal(std::string_view text,T& value,IntegerStyle style=IntegerStyle::None){
    using namespace Values;value=0;while(!text.empty()&&text.back()==Null)text.remove_suffix(1);
    if(style==IntegerStyle::Integer){while(!text.empty()&&Whitespace.find(text.front())!=std::string_view::npos)text.remove_prefix(1);while(!text.empty()&&Whitespace.find(text.back())!=std::string_view::npos)text.remove_suffix(1);}
    if(text.empty())return false;
    if(style==IntegerStyle::Integer&&text.front()==Plus){text.remove_prefix(1);if(text.empty()||text.front()<FirstDigit||text.front()>LastDigit)return false;}
    else if(text.front()<FirstDigit||text.front()>LastDigit){if(style!=IntegerStyle::Integer||text.front()!=Minus||!std::is_signed_v<T>)return false;}
    T parsed{};const auto result=std::from_chars(text.data(),text.data()+text.size(),parsed);if(result.ec!=std::errc{}||result.ptr!=text.data()+text.size())return false;value=parsed;return true;
}
template<class T> bool metadataNumber(const std::map<std::string,std::string>& metadata,const std::string& key,T& value,IntegerStyle style=IntegerStyle::None){value=0;const auto found=metadata.find(key);return found!=metadata.end()&&decimal(found->second,value,style);}
inline bool metadataInt(const std::map<std::string,std::string>& metadata,const std::string& key,std::int32_t& value,IntegerStyle style=IntegerStyle::None){return metadataNumber(metadata,key,value,style);}
inline bool metadataByte(const std::map<std::string,std::string>& metadata,const std::string& key,std::uint8_t& value){return metadataNumber(metadata,key,value);}
inline bool metadataUInt16(const std::map<std::string,std::string>& metadata,const std::string& key,std::uint16_t& value){return metadataNumber(metadata,key,value);}
inline std::int32_t readIntOrDefault(const std::map<std::string,std::string>& metadata,const std::string& key){std::int32_t value{};metadataInt(metadata,key,value,IntegerStyle::Integer);return value;}
}
