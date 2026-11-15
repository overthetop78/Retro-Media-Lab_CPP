#pragma once
#include <string>
#include <vector>
namespace RetroMediaLab::MediaFileSystems {
struct MessageRecipe {
    std::string resourceKey;
    std::vector<std::string> arguments;
    std::string translationDomain;
    bool operator==(const MessageRecipe&)const=default;
};
}
