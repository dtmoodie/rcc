#pragma once
#include <string>

namespace rcc
{
    struct InheritanceGraph;
    class InterfaceDatabase
    {
    public:
        static InterfaceDatabase* Instance();
        void RegisterInterface(const std::string& name,
                               unsigned int iid,
                               bool(*inheritance_f)(unsigned int),
                               bool(*direct_inheritance_f)(unsigned int));
        void GetInheritanceGraph(InheritanceGraph& graph);
    private:
        struct Impl;
        Impl* _pimpl;
        InterfaceDatabase();
        ~InterfaceDatabase();
    };

}
