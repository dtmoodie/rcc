#pragma once
#include <set>
#include <string>
#include <unordered_map>
namespace rcc
{
    struct InheritanceGraph
    {
        struct Interface
        {
            std::string name;
            std::set<unsigned int> parents;
            std::set<unsigned int> children;
        };
        std::unordered_map<unsigned int, Interface> interfaces;

    };
}
