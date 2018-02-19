#include "InterfaceDatabase.hpp"
#include <unordered_map>

namespace rcc
{


    void InterfaceDatabase::RegisterInterface(const std::string& name,
                           unsigned int iid,
                           bool(*inheritance_f)(unsigned int),
                           bool(*direct_inheritance_f)(unsigned int))
    {
        PerModuleInterface::GetInstance()->AddInterface(name, iid, inheritance_f, direct_inheritance_f);
    }

    void InterfaceDatabase::GetInheritanceGraph(InheritanceGraph& graph)
    {

    }
}
