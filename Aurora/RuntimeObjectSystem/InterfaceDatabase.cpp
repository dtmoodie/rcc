#include "InterfaceDatabase.hpp"
#include <unordered_map>

namespace rcc
{
    InterfaceDatabase* InterfaceDatabase::Instance()
    {
        static InterfaceDatabase g_inst;
        return &g_inst;
    }

    struct InterfaceDatabase::Impl
    {
        struct Info
        {
            std::string name;
            bool(*inheritance_f)(unsigned int);
            bool(*direct_inheritance_f)(unsigned int);

        };
        std::unordered_map<unsigned int, Info> m_info;
    };

    void InterfaceDatabase::RegisterInterface(const std::string& name,
                           unsigned int iid,
                           bool(*inheritance_f)(unsigned int),
                           bool(*direct_inheritance_f)(unsigned int))
    {
        _pimpl->m_info[iid] = {name, inheritance_f, direct_inheritance_f};
    }

    InterfaceDatabase::InterfaceDatabase()
    {
        _pimpl = new Impl();
    }

    InterfaceDatabase::~InterfaceDatabase()
    {
        delete _pimpl;
    }
}
