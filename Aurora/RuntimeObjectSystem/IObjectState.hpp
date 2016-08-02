#pragma once
#include "ObjectInterface.h"
#include "shared_ptr.hpp"


struct IObject;
struct IObjectConstructor;
template<class T> class TObjectConstructorConcrete;
template<class T> class TActual;
struct IObjectSharedState
{
    IObjectSharedState(IObject* obj, IObjectConstructor* constructor);
    ~IObjectSharedState();
    IObject* GetObject();
    rcc::shared_ptr<IObject> GetSharedPtr();
    rcc::weak_ptr<IObject> GetWeakPtr();
    void IncrementObject();
    void IncrementState();
    void DecrementObject();
    void DecrementState();
    template<class T> rcc::shared_ptr<T> GetSharedPtr()
    {
        return rcc::shared_ptr<T>(GetSharedPtr());
    }
    template<class T> rcc::weak_ptr<T> GetWeakPtr()
    {
        return rcc::weak_ptr<T>(GetSharedPtr());
    }
    int ObjectCount() const;
    int StateCount() const;
protected:
    friend struct IObject;
    friend struct IObjectConstructor;
    template<class T> friend class TObjectConstructorConcrete;
    template<class T> friend class TActual;

    void SetObject(IObject* object);
    void SetConstructor(IObjectConstructor* constructor);

    IObject* object;
    IObjectConstructor* constructor;
    int object_ref_count;
    int state_ref_count;
    PerTypeObjectId id;
};