#include "shared_ptr.hpp"
#include "IObject.h"
#include "ObjectInterface.h"

namespace rcc
{
    shared_ptr<IObject>::shared_ptr(IObject& obj)
    {
        const auto id = obj.GetPerTypeId();
        const auto ctr = obj.GetConstructor();
        m_control_block = ctr->GetControlBlock(id);
    }

    shared_ptr<IObject>::shared_ptr(std::shared_ptr<IObjectControlBlock> control_block):
        m_control_block(control_block)
    {

    }

    IObject* shared_ptr<IObject>::get() const
    {
        if(m_control_block)
        {
            return m_control_block->GetObject();
        }
        return nullptr;
    }

    
    IObject* shared_ptr<IObject>::operator->() const
    {
        return get();
    }

    
    IObject& shared_ptr<IObject>::operator*() const
    {
        return *get();
    }

    
    shared_ptr<IObject>::operator IObject*() const
    {
        return get();
    }

    
    shared_ptr<IObject>::operator bool() const
    {
        return m_control_block != nullptr;
    }

    
    std::shared_ptr<IObjectControlBlock> shared_ptr<IObject>::GetControlBlock() const
    {
        return m_control_block;
    }


    weak_ptr<IObject>::weak_ptr(std::weak_ptr<IObjectControlBlock> control_block):
            m_control_block(control_block){}

    shared_ptr<IObject> weak_ptr<IObject>::lock() const
    {
        return shared_ptr<IObject>(m_control_block.lock());
    }
}