#include "WriterInternal.h"
#include "../../../Primitives/Petscii.h"
#include <QString>
namespace RetroMediaLab::MediaFileSystems::CommodoreDos {
WritePolicy::WritePolicy():WritePolicy(Constants::Prg,Constants::DefaultRecordLength){}
WritePolicy::WritePolicy(std::uint8_t type,std::uint8_t length):type_(type|Constants::Closed),recordLength_(length){
    const auto base=type&Constants::BaseTypeMask;
    if(base<Constants::Seq||base>Constants::Rel)throw MediaError(DiagnosticCode::CbmWriteType,{std::to_string(type)});
    if(base==Constants::Rel&&length==0)throw MediaError(DiagnosticCode::CbmWriteRecordLength);
}
namespace Writing {
std::vector<std::uint8_t> nameBytes(std::string_view name){
    const auto value=QString::fromUtf8(name.data(),qsizetype(name.size())).toStdU16String();
    return Petscii::encode(value,Constants::NameLength);
}
}
bool validName(std::string_view name){
    const auto value=QString::fromUtf8(name.data(),qsizetype(name.size()));
    if(value.trimmed().isEmpty()||value.size()>Constants::NameLength)return false;
    try{Writing::nameBytes(name);return true;}
    catch(const MediaError& error){switch(error.diagnostic().code){case DiagnosticCode::PetsciiCharacter:case DiagnosticCode::PetsciiTooLong:return false;default:throw;}}
}
}
