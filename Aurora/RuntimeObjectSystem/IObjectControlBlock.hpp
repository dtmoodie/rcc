#ifndef RCC_IOBJECT_CONTROL_BLOCK_HPP
#define RCC_IOBJECT_CONTROL_BLOCK_HPP

#include "IObject.h"

#include <memory>

struct IObject;
// When a Object factory creates an object, it creates a control block for that object
// The control block doesn't change accross recompiles, and the constructor can update the control block with 
// a new pointer to the object.
// The goal of this is to get rid of the previous IObjectState stuffs for something that works using std::shared_ptr and std::weak_ptr
struct IObjectControlBlock
{
    virtual ~IObjectControlBlock();
    // Fetch the object
    virtual IObject* GetObject() const = 0;
    // Set the object, used by the constructor on a recompile
    virtual void SetObject(IObject* obj) = 0;
    virtual void DeleteObject() = 0;
};

template<class T, class U, class E = void>
struct TObjectControlBlockImpl;

template<class T>
struct TObjectControlBlock: TObjectControlBlockImpl<T, typename T::BaseTypes>
{
    using Super = TObjectControlBlockImpl<T, typename T::BaseTypes>;
    using Super::GetTypedObject;

    TObjectControlBlock(T* obj = nullptr):
        Super(obj)
    {
        this->SetObject(obj);
    }

    void GetTypedObject(T** ret) const
    {
        *ret = m_obj;
    }

    void SetObject(IObject* obj) override
    {
        Super::SetObject(obj);
        m_obj = dynamic_cast<T*>(obj);
    }

    void SetTypedObject(T* obj)
    {
        m_obj = obj;
    }
private:
    T* m_obj = nullptr;
};

template<class T, class U, class E>
struct TObjectControlBlockImpl: TObjectControlBlock<U>
{
    TObjectControlBlockImpl(T* obj):
        TObjectControlBlock<U>(obj)
    {
        
    }
};

template<>
struct TObjectControlBlock<IObject>: IObjectControlBlock
{
    TObjectControlBlock(IObject* obj = nullptr)
    {
        m_obj = obj;
    }

    ~TObjectControlBlock()
    {
        // Only delete it from this specialization
        delete m_obj;
    }

    IObject* GetObject() const override
    {
        return m_obj;
    }

    void SetObject(IObject* obj) override
    {
        m_obj = obj;
    }

    void DeleteObject() override
    {
        delete m_obj;
        m_obj = nullptr;
    }

    void GetTypedObject(IObject** ret)
    {
        *ret = m_obj;
    }
    void SetTypedObject(IObject* obj)
    {
        m_obj = obj;
    }
private:
    IObject* m_obj = nullptr;
};

#endif // RCC_IOBJECT_CONTROL_BLOCK_HPP
