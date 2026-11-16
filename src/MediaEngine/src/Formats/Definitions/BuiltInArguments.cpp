#include <RetroMediaLab/MediaEngine/BuiltInDiskDefinitions.h>
#include "BuiltInConstants.h"
#include "../Catalog/Constants.h"
#include "../../Primitives/Identifiers.h"
#include <algorithm>
namespace RetroMediaLab::MediaEngine {
bool supportsBuiltInDiskDefinition(std::string_view id){
    return std::any_of(CatalogConstants::BuiltInDefinitions.begin(),CatalogConstants::BuiltInDefinitions.end(),[&](auto known){return Primitives::ordinalIdentifierEqualsIgnoreCase(id,known);});
}
std::filesystem::path builtInDiskDefinitionPath(const std::filesystem::path& directory){
    namespace C=BuiltInDefinitionConstants;
    return directory/std::string(C::AssetsDirectory)/std::string(C::DefinitionsDirectory)/std::string(C::FileName);
}
void appendBuiltInDiskDefinitionArguments(std::vector<std::string>& arguments,std::string_view formatId,std::span<const EnabledOption> options,const std::filesystem::path& directory){
    if(!supportsBuiltInDiskDefinition(formatId)||std::any_of(options.begin(),options.end(),[](const auto& option){return Primitives::ordinalIdentifierEqualsIgnoreCase(option.argument,BuiltInDefinitionConstants::Argument);}))return;
    const auto path=builtInDiskDefinitionPath(directory).u8string();
    std::string argument(BuiltInDefinitionConstants::Argument),value(path.begin(),path.end());
    arguments.reserve(arguments.size()+BuiltInDefinitionConstants::ArgumentCount);arguments.push_back(std::move(argument));arguments.push_back(std::move(value));
}
}
