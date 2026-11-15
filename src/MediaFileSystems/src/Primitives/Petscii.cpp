#include "Petscii.h"
#include "PetsciiConstants.h"
#include <QChar>
#include <QString>
namespace RetroMediaLab::MediaFileSystems::Petscii {
std::string decode(Bytes bytes) {
    using namespace Constants;std::string result;
    for(const auto raw:bytes) {
        if(raw==Terminator||raw==Padding)break;
        const auto value=std::uint8_t(raw&Mask);
        if(value>=DirectStart&&value<=DirectEnd)result.push_back(char(value));
        else if(value>=LowerStart&&value<=LowerEnd)result.push_back(char(value-CaseOffset));
        else result+=Replacement;
    }
    const auto first=result.find_first_not_of(char(DirectStart));
    if(first==std::string::npos)return {};
    return result.substr(first,result.find_last_not_of(char(DirectStart))-first+1);
}
std::vector<std::uint8_t> encode(std::u16string_view value,std::int32_t length) {
    using namespace Constants;
    if(length<0)throw MediaError(DiagnosticCode::PetsciiLength,{std::to_string(length)});
    if(value.size()>std::uint32_t(length))throw MediaError(DiagnosticCode::PetsciiTooLong,{std::to_string(length)});
    std::vector<std::uint8_t> result(std::size_t(length),Padding);
    for(std::size_t index=0;index<value.size();++index) {
        auto character=value[index];
        if(character>=AsciiLowerStart&&character<=AsciiLowerEnd)character=char16_t(character-CaseOffset);
        else if(character==LongS)character=UpperS;
        if(character<DirectStart||character>DirectEnd)throw MediaError(DiagnosticCode::PetsciiCharacter,{QString(QChar(value[index])).toUtf8().toStdString()});
        result[index]=std::uint8_t(character);
    }
    return result;
}
}
