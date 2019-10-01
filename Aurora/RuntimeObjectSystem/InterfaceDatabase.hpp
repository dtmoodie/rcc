#ifndef RCC_INTERFACE_DATABASE_HPP
#define RCC_INTERFACE_DATABASE_HPP

#include "ObjectInterfacePerModule.h"

#include <string>

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

#endif // RCC_INTERFACE_DATABASE_HPP