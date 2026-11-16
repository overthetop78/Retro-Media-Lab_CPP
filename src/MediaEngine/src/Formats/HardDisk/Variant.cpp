#include "Variant.h"
#include "ContainerConstants.h"
#include "../../Primitives/Identifiers.h"
#include <QString>
#include <algorithm>
namespace RetroMediaLab::MediaEngine::HardDiskContainers {
bool selectFixedVariant(const MediaDocument& document,const BlockImage& blocks,std::string_view format){
    using namespace Constants;const auto found=document.metadata().find(std::string(VariantKey));
    if(Primitives::ordinalIdentifierEqualsIgnoreCase(document.formatId(),format)&&found!=document.metadata().end()){
        const auto text=QString::fromUtf8(found->second).trimmed();const auto value=text.toUtf8().toStdString();bool numeric=false;const auto number=text.toInt(&numeric);
        if(Primitives::ordinalIdentifierEqualsIgnoreCase(value,FixedVariant)||(numeric&&number==FixedEnumValue))return true;
        if(Primitives::ordinalIdentifierEqualsIgnoreCase(value,DynamicVariant)||(numeric&&number==DynamicEnumValue))return false;
    }
    return std::none_of(blocks.ranges().begin(),blocks.ranges().end(),[](const auto& range){return range.kind()==DataRangeKind::Zero||range.kind()==DataRangeKind::Unallocated;});
}
}
