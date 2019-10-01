#ifndef RCC_IOBJECT_CONTROL_BLOCK_HPP
#define RCC_IOBJECT_CONTROL_BLOCK_HPP

#include "IObject.h"
#include "ObjectInterface.h"

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

template<class T>
struct TObjectControlBlock: IObjectControlBlock
{
    TObjectControlBlock(T* obj = nullptr):
        m_obj(obj)
    {

    }

    ~TObjectControlBlock() override
    {
        delete m_obj;
    }

    IObject* GetObject() const override
    {
        return m_obj;
    }

    T* GetTypedObject() const
    {
        return m_obj;
    }

    void SetObject(IObject* obj) override
    {
        m_obj = dynamic_cast<T*>(obj);
    }

    void SetTypedObject(T* obj)
    {
        m_obj = obj;
    }

    void DeleteObject() override
    {
        delete m_obj;
        m_obj = nullptr;
    }
private:
    T* m_obj = nullptr;
};

#endif // RCC_IOBJECT_CONTROL_BLOCK_HPP