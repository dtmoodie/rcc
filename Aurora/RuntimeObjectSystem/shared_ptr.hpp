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

        template<class ... ARGS>
        static shared_ptr create(ARGS&&... args);

        shared_ptr(T& obj);

        shared_ptr();
        shared_ptr(std::shared_ptr<IObjectControlBlock> control_block);
        
        shared_ptr(const shared_ptr<IObject>& sp);

        template<class U>
        shared_ptr(const shared_ptr<U>& sp);

        template<class U>
        shared_ptr<U> DynamicCast() const;
        
        T* get() const;

        T* operator->() const;

        T& operator*() const;

        operator T*() const;

        operator bool() const;

        template<class U>
        bool operator ==(const shared_ptr<U>& other) const;
        template<class U>
        bool operator !=(const shared_ptr<U>& other) const;

        std::shared_ptr<IObjectControlBlock> GetControlBlock() const;
        void reset();
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
        weak_ptr(T& obj);

        weak_ptr(const shared_ptr<T>& sp);
        template<class U>
        weak_ptr(const shared_ptr<U>& sp);

        shared_ptr<T> lock() const;

        template<class U>
        bool operator ==(const weak_ptr<U>& other) const;
        template<class U>
        bool operator !=(const weak_ptr<U>& other) const;
        template<class U>
        bool operator ==(const shared_ptr<U>& other) const;
        template<class U>
        bool operator !=(const shared_ptr<U>& other) const;

        bool operator ==(const T* obj) const;

    private:
        std::weak_ptr<IObjectControlBlock> m_control_block;
    };

    /////////////////////////////////////////////////////////////////////
    // Implementation

    template<class T>
    template<class ... ARGS>
    shared_ptr<T> shared_ptr<T>::create(ARGS&& ... args)
    {
        return T::create(std::forward<ARGS>(args)...);
    }

    template<class T>
    shared_ptr<T>::shared_ptr(T& obj)
    {
        const auto id = obj.GetPerTypeId();
        const auto ctr = obj.GetConstructor();
        SetControlBlock(ctr->GetControlBlock(id));
    }

    template<class T> 
    shared_ptr<T>::shared_ptr()
    {

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
    template<class U>
    shared_ptr<U> shared_ptr<T>::DynamicCast() const
    {
        return shared_ptr<T>(std::dynamic_pointer_cast<TObjectControlBlock<U>>(GetControlBlock()));
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
    template<class U>
    bool shared_ptr<T>::operator ==(const shared_ptr<U>& other) const
    {
        return m_control_block == other.m_control_block;
    }

    template<class T>
    template<class U>
    bool shared_ptr<T>::operator !=(const shared_ptr<U>& other) const
    {
        return !(*this == other);
    }

    template<class T>
    std::shared_ptr<IObjectControlBlock> shared_ptr<T>::GetControlBlock() const
    {
        return m_control_block;
    }

    template<class T>
    void shared_ptr<T>::SetControlBlock(std::shared_ptr<IObjectControlBlock> block_)
    {
        auto typed = std::dynamic_pointer_cast<TObjectControlBlock<T>>(block_);
        m_control_block = std::move(typed);
    }

    template<class T>
    void shared_ptr<T>::reset()
    {
        m_control_block.reset();
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
    weak_ptr<T>::weak_ptr(T& obj)
    {
        const auto id = obj.GetPerTypeId();
        const auto ctr = obj.GetConstructor();
        m_control_block = ctr->GetControlBlock(id);
    }

    template<class T>
    template<class U>
    weak_ptr<T>::weak_ptr(const shared_ptr<U>& sp):
        m_control_block(sp.GetControlBlock())
    {
        static_assert(std::is_base_of<T, U>::value || std::is_base_of<U, T>::value, "Must derive one way or the other from this type");
    }

    template<class T>
    template<class U>
    bool weak_ptr<T>::operator ==(const weak_ptr<U>& other) const
    {
        auto l1 = m_control_block.lock();
        auto l2 = other.m_control_block.lock();
        return l1 == l1;
    }

    template<class T>
    template<class U>
    bool weak_ptr<T>::operator !=(const weak_ptr<U>& other) const
    {
        return !(*this == other);
    }

    template<class T>
    template<class U>
    bool weak_ptr<T>::operator ==(const shared_ptr<U>& other) const
    {
        auto cb = other.GetControlBlock();
        auto cb1 = m_control_block.lock();
        return cb == cb1;
    }

    template<class T>
    template<class U>
    bool weak_ptr<T>::operator !=(const shared_ptr<U>& other) const
    {
        return !(*this == other);
    }

    template<class T>
    bool weak_ptr<T>::operator ==(const T* obj) const
    {
        auto s = lock();
        return s == obj;
    }

    template<class T>
    shared_ptr<T> weak_ptr<T>::lock() const
    {
        return shared_ptr<T>(m_control_block.lock());
    }


}
#endif // RCC_SHARED_PTR_HPP
