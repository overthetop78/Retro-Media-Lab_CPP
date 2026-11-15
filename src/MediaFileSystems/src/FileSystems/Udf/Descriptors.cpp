#include "Internal.h"
#include <charconv>
#include <chrono>
namespace RetroMediaLab::MediaFileSystems::Udf {
void validate(Bytes bytes,std::uint16_t expected,std::optional<std::uint32_t> expectedLocation){
    if(bytes.size()<DescriptorTagSize)throw MediaError(DiagnosticCode::UdfTagIncomplete);
    const auto id=little<std::uint16_t>(bytes,DescriptorTagIdOffset);if(id!=expected)throw MediaError(DiagnosticCode::UdfTagId,{std::to_string(expected),std::to_string(id)});
    const auto location=little<std::uint32_t>(bytes,DescriptorTagLocationOffset);if(expectedLocation&&location!=*expectedLocation)throw MediaError(DiagnosticCode::UdfTagLocation,{std::to_string(location),std::to_string(*expectedLocation)});
    std::uint8_t checksum=0;for(std::size_t i=0;i<DescriptorTagSize;++i)if(i!=DescriptorTagChecksumOffset)checksum=std::uint8_t(checksum+bytes[i]);
    if(checksum!=bytes[DescriptorTagChecksumOffset])throw MediaError(DiagnosticCode::UdfTagChecksum);
    const auto length=little<std::uint16_t>(bytes,DescriptorTagCrcLengthOffset);if(length>bytes.size()-DescriptorTagSize)throw MediaError(DiagnosticCode::UdfCrcLength);
    std::uint16_t crc=0;for(const auto byte:bytes.subspan(DescriptorTagSize,length)){crc^=std::uint16_t(byte<<ByteShift);for(int bit=0;bit<ByteShift;++bit)crc=std::uint16_t((crc&CrcHighBit)?(crc<<1)^CrcPolynomial:crc<<1);}
    if(crc!=little<std::uint16_t>(bytes,DescriptorTagCrcOffset))throw MediaError(DiagnosticCode::UdfCrc);
}
std::string compressedName(Bytes bytes){
    if(bytes.empty())return {};const auto id=bytes.front();bytes=bytes.subspan(1);QString text;
    if(id==CompressedUnicode8)text=QString::fromLatin1(QByteArrayView(reinterpret_cast<const char*>(bytes.data()),qsizetype(bytes.size())));
    else if(id==CompressedUnicode16){if(bytes.size()%UnicodeCodeUnitLength)throw MediaError(DiagnosticCode::UdfUnicodeOdd);text=Optical::utf16be(bytes);}
    else throw MediaError(DiagnosticCode::UdfCompression,{std::to_string(id)});
    while(!text.isEmpty()&&text.back()==QChar(Null))text.chop(1);return text.toUtf8().toStdString();
}
std::string dstring(Bytes bytes){if(bytes.empty()||!bytes.back())return {};const auto length=bytes.back();if(length>=bytes.size())throw MediaError(DiagnosticCode::UdfDStringLength);return compressedName(bytes.first(length));}
std::string revision(std::uint16_t value){const auto major=QString::number(value>>ByteShift,HexBase).toUpper(),minor=QString::number(value&ByteMask,HexBase).toUpper().rightJustified(RevisionMinorDigits,QChar(RevisionZero));return (major+QString::fromLatin1(RevisionDot.data(),qsizetype(RevisionDot.size()))+minor).toStdString();}
std::optional<Timestamp> timestamp(Bytes bytes,std::size_t offset){
    if(offset>bytes.size()||TimestampSize>bytes.size()-offset)return {};const auto data=bytes.subspan(offset,TimestampSize);const auto year=little<std::uint16_t>(data,TimestampYearOffset);
    if(!year||!data[TimestampMonthOffset]||!data[TimestampDayOffset])return {};
    const auto zone=little<std::uint16_t>(data,0)&TimestampTimezoneMask;const auto minutes=zone==TimestampUnspecifiedTimezone?0:zone>=TimezoneNegative?int(zone)-TimezoneModulus:int(zone);
    Timestamp result;result.year=year;result.month=data[TimestampMonthOffset];result.day=data[TimestampDayOffset];result.hour=data[TimestampHourOffset];result.minute=data[TimestampMinuteOffset];result.second=data[TimestampSecondOffset];result.offsetMinutes=minutes;if(!validTimestamp(result))return {};
    using namespace std::chrono;
    const auto fraction=std::int64_t(data[TimestampCentisecondsOffset])*CentisecondNanoseconds+std::int64_t(data[TimestampHundredsOfMicrosecondsOffset])*HundredMicrosecondNanoseconds+std::int64_t(data[TimestampMicrosecondsOffset])*MicrosecondNanoseconds;
    const auto local=sys_days(std::chrono::year(result.year)/month(unsigned(result.month))/day(unsigned(result.day)))+hours(result.hour)+std::chrono::minutes(result.minute)+seconds(result.second)+seconds(fraction/NanosecondsPerSecond);
    const auto dayValue=floor<days>(local);const year_month_day date(dayValue);const hh_mm_ss time(local-dayValue);
    const auto utc=local-std::chrono::minutes(minutes);const auto utcYear=int(year_month_day(floor<days>(utc)).year());
    if(utcYear<MinimumYear||utcYear>MaximumYear)return {};
    result.year=int(date.year());result.month=int(unsigned(date.month()));result.day=int(unsigned(date.day()));result.hour=int(time.hours().count());result.minute=int(time.minutes().count());result.second=int(time.seconds().count());result.nanosecond=int(fraction%NanosecondsPerSecond);return validTimestamp(result)?std::optional(result):std::nullopt;
}
}
