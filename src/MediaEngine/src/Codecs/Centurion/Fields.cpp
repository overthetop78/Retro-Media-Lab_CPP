#include "Mfm.h"
#include "../Crc16.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Centurion {
std::vector<std::uint8_t> buildHeader(std::uint8_t cylinder,std::uint8_t sector,std::stop_token token){const std::array identity{cylinder,sector};return Crc16::append(identity,Crc16::Constants::Ccitt,Crc16::Constants::Zero,token);}
std::vector<std::uint8_t> buildDataField(MediaFileSystems::Bytes data,std::stop_token token){
 using namespace Constants;checkCancellation(token);const auto blocks=std::max(std::size_t(MinimumBlockCount),data.size()/AllocationBlockSize+std::size_t(data.size()%AllocationBlockSize!=0));
 if(blocks>MaximumBlockCount)throw MediaError(DiagnosticCode::CenturionAddressRange,{std::string(BlockCountField),std::to_string(blocks),std::to_string(MaximumBlockCount)});
 std::vector<std::uint8_t> protectedBytes;protectedBytes.reserve(DataCrcPrefixByteCount+blocks*AllocationBlockSize);protectedBytes.push_back(std::uint8_t(blocks));protectedBytes.push_back(SupportedDataKey);protectedBytes.insert(protectedBytes.end(),data.begin(),data.end());protectedBytes.resize(DataCrcPrefixByteCount+blocks*AllocationBlockSize,PaddingByte);auto result=Crc16::append(protectedBytes,Crc16::Constants::Ccitt,Crc16::Constants::Zero,token);result.insert(result.begin(),ReservedPrefix);return result;
}
}
