#ifndef RCC_SHARED_PTR_HPP
#define RCC_SHARED_PTR_HPP
#include "IObjectControlBlock.hpp"

#include <memory>

struct IObjectControlBlock;
template<class T>
struct TObjectControlBlock;
struct IObject;

namespace rcc
{
    template<class T>
    struct shared_ptr
    {
        using element_type = T;

        static shared_ptr create();

        shared_ptr(T& obj);

        shared_ptr(std::shared_ptr<IObjectControlBlock> control_block = {});
        
        shared_ptr(const shared_ptr<IObject>& sp);

        template<class U>
        shared_ptr(const shared_ptr<U>& sp);
        
        T* get() const;

        T* operator->() const;

        T& operator*() const;

        operator T*() const;

        operator bool() const;

        std::shared_ptr<IObjectControlBlock> GetControlBlock() const;
    private:
        void SetControlBlock(std::shared_ptr<IObjectControlBlock>);
        std::shared_ptr<TObjectControlBlock<T>> m_control_block;
    };


    template<class T>
    struct weak_ptr
    {
        using element_type = T;
        weak_ptr(std::weak_ptr<IObjectControlBlock> control_block = {});
        weak_ptr(std::shared_ptr<IObjectControlBlock> control_block);

        weak_ptr(const shared_ptr<T>& sp);

        shared_ptr<T> lock() const;
    private:
        std::weak_ptr<IObjectControlBlock> m_control_block;
    };

    /////////////////////////////////////////////////////////////////////
    // Implementation

    template<class T>
    shared_ptr<T> shared_ptr<T>::create()
    {
        return T::create();
    }

    template<class T>
    shared_ptr<T>::shared_ptr(T& obj)
    {
        const auto id = obj.GetPerTypeId();
        const auto ctr = obj.GetConstructor();
        SetControlBlock(ctr->GetControlBlock(id));
    }

    template<class T>
    shared_ptr<T>::shared_ptr(std::shared_ptr<IObjectControlBlock> control_block)
    {
        SetControlBlock(control_block);
    }

    template<class T>
    shared_ptr<T>::shared_ptr(const shared_ptr<IObject>& sp)
    {
        SetControlBlock(sp.GetControlBlock());
    }

    template<class T>
    template<class U>
    shared_ptr<T>::shared_ptr(const rcc::shared_ptr<U>& sp)
    {
        SetControlBlock(sp.GetControlBlock());
    }
    
    template<class T>
    T* shared_ptr<T>::get() const
    {
        if(m_control_block)
        {
            T* ptr = nullptr;
            m_control_block->GetTypedObject(&ptr);
            return ptr;
        }
        return nullptr;
    }

    template<class T>
    T* shared_ptr<T>::operator->() const
    {
        return get();
    }

    template<class T>
    T& shared_ptr<T>::operator*() const
    {
        return *get();
    }

    template<class T>
    shared_ptr<T>::operator T*() const
    {
        return get();
    }

    template<class T>
    shared_ptr<T>::operator bool() const
    {
        return m_control_block != nullptr;
    }

    template<class T>
    std::shared_ptr<IObjectControlBlock> shared_ptr<T>::GetControlBlock() const
    {
        return m_control_block;
    }

    template<class T>
    void shared_ptr<T>::SetControlBlock(std::shared_ptr<IObjectControlBlock> block_)
    {
        m_control_block = std::dynamic_pointer_cast<TObjectControlBlock<T>>(block_);
    }

    template<class T>
    weak_ptr<T>::weak_ptr(std::weak_ptr<IObjectControlBlock> control_block):
        m_control_block(control_block){}

    template<class T>
    weak_ptr<T>::weak_ptr(std::shared_ptr<IObjectControlBlock> control_block):
        m_control_block(control_block){}

    template<class T>
    weak_ptr<T>::weak_ptr(const shared_ptr<T>& sp):
        m_control_block(sp.GetControlBlock())
    {
        
    }

    template<class T>
    shared_ptr<T> weak_ptr<T>::lock() const
    {
        return shared_ptr<T>(m_control_block.lock());
    }

}
#endif // RCC_SHARED_PTR_HPP
