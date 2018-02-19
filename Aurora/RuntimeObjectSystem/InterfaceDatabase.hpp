#pragma once
#include <string>
#include "ObjectInterfacePerModule.h"


namespace rcc
{
    struct InheritanceGraph;
    class InterfaceDatabase
    {
    public:
        static void RegisterInterface(const std::string& name,
                               unsigned int iid,
                               bool(*inheritance_f)(unsigned int),
                               bool(*direct_inheritance_f)(unsigned int));
        static void GetInheritanceGraph(InheritanceGraph& graph);
    };

}
