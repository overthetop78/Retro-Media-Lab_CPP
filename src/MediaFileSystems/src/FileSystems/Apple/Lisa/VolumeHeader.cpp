#include "Internal.h"
#include <QString>
namespace RetroMediaLab::MediaFileSystems::Lisa {
namespace Internal {
std::string hex(std::uint16_t value){return QString::number(value,Constants::HexBase).rightJustified(Constants::HexWordWidth,QChar(Constants::HexZero)).toUpper().toStdString();}
void append(std::vector<std::uint8_t>& target,Bytes data){if(data.size()>Constants::MaximumBuffer-target.size())throw MediaError(DiagnosticCode::ReadOutOfBounds);target.insert(target.end(),data.begin(),data.end());}
Mddf mddf(const std::vector<Page>& pages){
    std::vector<const Page*> selected;for(const auto& page:pages)if(page.tag&&page.tag->fileId==Constants::MddfFileId)selected.push_back(&page);
    if(selected.empty())throw MediaError(DiagnosticCode::LisaMissingTaggedSystem,{std::to_string(pages.size())});
    std::stable_sort(selected.begin(),selected.end(),[](const auto* a,const auto* b){return a->tag->pageNumber<b->tag->pageNumber;});
    const auto& data=selected.back()->block->data;if(data.size()<Constants::MinimumMddfLength)throw MediaError(DiagnosticCode::LisaTruncatedMddf,{std::to_string(data.size()),std::to_string(Constants::MinimumMddfLength)});
    const auto version=Binary::big<std::uint16_t>(data,Constants::VersionOffset);const auto length=std::min(std::size_t(data[Constants::NameLengthOffset]),Constants::MaximumName);
    auto name=length>0&&Constants::NameOffset+length<=data.size()?Apple::decodeLisaName(Bytes(data).subspan(Constants::NameOffset,length)):std::string{};
    if(Identifiers::isBlank(name))name=Constants::DefaultVolumeName;return {version,std::move(name)};
}
}
std::string catalogVersionName(std::uint16_t value){
    switch(CatalogVersion(value)){case CatalogVersion::Table:return std::string(Constants::TableName);case CatalogVersion::Hash:return std::string(Constants::HashName);case CatalogVersion::BTree:return std::string(Constants::BTreeName);default:return std::string(Constants::UnknownVersionPrefix)+Internal::hex(value);}
}
}
