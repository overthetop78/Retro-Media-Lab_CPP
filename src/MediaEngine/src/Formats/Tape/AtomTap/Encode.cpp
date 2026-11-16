#include "Codec.h"
#include "Constants.h"
#include <RetroMediaLab/MediaEngine/Diagnostics.h>
#include <QtEndian>
#include <QString>
#include <algorithm>
#include <charconv>
namespace RetroMediaLab::MediaEngine::AtomTap {
using namespace Constants;
std::vector<std::uint8_t> encode(std::span<const SequentialDecodedBlock> files,std::stop_token token){
    if(files.empty())throw MediaError(DiagnosticCode::AtomTapFiles);std::vector<std::uint8_t> output;
    for(const auto& file:files){checkCancellation(token);const auto& metadata=file.metadata();const auto found=metadata.find(std::string(FileNameKey));unsigned load{},execute{};const auto address=[&](std::string_view key,unsigned& value){const auto item=metadata.find(std::string(key));if(item==metadata.end())return false;const auto text=QString::fromUtf8(item->second).trimmed().toStdString();const auto result=std::from_chars(text.data(),text.data()+text.size(),value,HexRadix);return result.ec==std::errc{}&&result.ptr==text.data()+text.size()&&value<=MaximumFileLength;};
        if(file.integrityValid()!=true||file.data().size()>MaximumFileLength||found==metadata.end()||found->second.empty()||found->second.size()>FileNameBytes||std::any_of(found->second.begin(),found->second.end(),[](unsigned char value){return value<FirstPrintable||value>LastPrintable;})||!address(LoadKey,load)||!address(ExecuteKey,execute))throw MediaError(DiagnosticCode::AtomTapFiles);
        if(FileHeaderBytes+file.data().size()>MaximumBuffer-output.size())throw MediaError(DiagnosticCode::AtomTapLimit);const auto offset=output.size();output.resize(offset+FileHeaderBytes);std::copy(found->second.begin(),found->second.end(),output.begin()+offset);qToLittleEndian(static_cast<std::uint16_t>(load),output.data()+offset+LoadAddressOffset);qToLittleEndian(static_cast<std::uint16_t>(execute),output.data()+offset+ExecutionAddressOffset);qToLittleEndian(static_cast<std::uint16_t>(file.data().size()),output.data()+offset+LengthOffset);output.insert(output.end(),file.data().begin(),file.data().end());
    }return output;
}
}
