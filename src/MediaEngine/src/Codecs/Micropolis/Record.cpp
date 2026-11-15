#include "Mfm.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::Micropolis {
std::uint8_t checksum(MediaFileSystems::Bytes data,std::stop_token token){checkCancellation(token);std::int32_t value=0;for(auto byte:data){checkCancellation(token);if(value>Constants::ChecksumModulus)value-=Constants::ChecksumModulus;value+=byte;}return std::uint8_t(value);}
bool Record::checksumValid(std::stop_token token)const {checkCancellation(token);const auto start=std::min(std::size_t(Constants::ChecksumDataOffset),bytes.size());const auto count=std::min(std::size_t(Constants::ChecksumDataLength),bytes.size()-start);return checksum(MediaFileSystems::Bytes(bytes).subspan(start,count),token)==storedChecksum;}
std::optional<Record> parseRecord(MediaFileSystems::Bytes bytes,std::stop_token token){
 using namespace Constants;checkCancellation(token);if(bytes.size()!=RecordByteCount||bytes[AddressMarkOffset]!=AddressMark)return {};
 return Record{bytes[CylinderOffset],bytes[SectorOffset],bytes[ChecksumOffset],std::vector<std::uint8_t>(bytes.begin()+DataOffset,bytes.begin()+ChecksumOffset),std::vector<std::uint8_t>(bytes.begin()+TrailerOffset,bytes.end()),std::vector<std::uint8_t>(bytes.begin(),bytes.end())};
}
Record createRecord(std::uint8_t cylinder,std::uint8_t sector,MediaFileSystems::Bytes data,std::stop_token token){
 using namespace Constants;checkCancellation(token);if(data.size()>MaximumRecordInputSize)throw MediaError(DiagnosticCode::MicropolisRecordInput,{std::to_string(data.size()),std::to_string(MaximumRecordInputSize)});
 std::vector<std::uint8_t> bytes(RecordByteCount);bytes[AddressMarkOffset]=AddressMark;bytes[CylinderOffset]=cylinder;bytes[SectorOffset]=sector;std::copy(data.begin(),data.end(),bytes.begin()+DataOffset);bytes[ChecksumOffset]=checksum(MediaFileSystems::Bytes(bytes).subspan(ChecksumDataOffset,ChecksumDataLength),token);return *parseRecord(bytes,token);
}
}
