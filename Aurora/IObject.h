//
// Copyright (c) 2010-2011 Matthew Jack and Doug Binks
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// IObject header file.
//
// The RuntimeCompiler library does not declare an IObject interface, only forward declares it.
// Hence each project can define their own base interface for objects they want to runtime compile
// and construct by using their own declaration of IObject in their own header file.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef IOBJECT_INCLUDED
#define IOBJECT_INCLUDED
#include "RuntimeObjectSystem/ObjectInterface.h"
#include <ct/String.hpp>
#include <iostream>
#include <algorithm>
#include <assert.h>

struct ISimpleSerializer;
class ObjectFactorySystem;
namespace rcc
{
    template<typename T> class shared_ptr;
    template<typename T> class weak_ptr;
}

typedef unsigned int InterfaceID;
#ifdef _MSC_VER
#define INTERFACE_HASH static constexpr InterfaceID class_hash() {return hashClassName(__FUNCTION__);}
#else
#define INTERFACE_HASH static constexpr InterfaceID class_hash() {return hashClassName(__PRETTY_FUNCTION__);}
#endif

// Template to help with IIDs
template< typename TInferior, typename TSuper> struct TInterface : public TSuper
{
    static constexpr uint32_t getHash() { return ct::hashClassName(__FUNCTION__); }
    static const InterfaceID s_interfaceID = getHash();
    static std::string GetInterfaceName()
    {
#ifdef _MSC_VER
        return std::string(__FUNCTION__).substr(ct::findFirst(__FUNCTION__, ' ') + 1,
            ct::findFirst(__FUNCTION__, ',') - ct::findFirst(__FUNCTION__, ' ') - 1);
#else
        std::string str = __PRETTY_FUNCTION__;
        auto pos1 = str.find("TInferior = ");
        return str.substr(pos1 + 12, str.find(';', pos1+13) - pos1 - 12);
#endif
    }
    virtual IObject* GetInterface( InterfaceID _iid)
    {
        switch(_iid)
        {
        case s_interfaceID:
            return this;
        default:
            return TSuper::GetInterface(_iid);
        }
    }
};

// IObject itself below is a special case as the base class
// Also it doesn't hurt to have it coded up explicitly for reference
struct IObject
{
    static const InterfaceID s_interfaceID = ct::ctcrc32("IObject");

    virtual IObject* GetInterface(InterfaceID __iid)
    {
        switch(__iid)
        {
        case s_interfaceID:
            return this;
        default:
            return nullptr;
        }
    }

    template< typename T> void GetInterface( T** pReturn )
    {
        GetInterface( T::s_interfaceID, (void**)pReturn );
    }

    static std::string GetInterfaceName()
    {
        return "IObject";
    }

    IObject() : _isRuntimeDelete(false) {}
    virtual ~IObject()
    {

    }

    // Perform any object initialization
    // Should be called with isFirstInit=true on object creation
    // Will automatically be called with isFirstInit=false whenever a system serialization is performed
    virtual void Init( bool isFirstInit )
    {

    }

    //return the PerTypeObjectId of this object, which is unique per class
    virtual PerTypeObjectId GetPerTypeId() const = 0;

    virtual void GetObjectId( ObjectId& id ) const
    {
        id.m_ConstructorId = GetConstructor()->GetConstructorId();
        id.m_PerTypeId = GetPerTypeId();
    }
    virtual ObjectId GetObjectId() const
    {
        ObjectId ret;
        GetObjectId( ret );
        return ret;
    }


    //return the constructor for this class
    virtual IObjectConstructor* GetConstructor() const = 0;

    //serialise is not pure virtual as many objects do not need state
    virtual void Serialize(ISimpleSerializer *pSerializer)
    {


    }

    virtual const char* GetTypeName() const = 0;

protected:
    bool IsRuntimeDelete() { return _isRuntimeDelete; }

private:
    friend class ObjectFactorySystem;

    // Set to true when object is being deleted because a new version has been created
    // Destructor should use this information to not delete other IObjects in this case
    // since these objects will still be needed
    bool _isRuntimeDelete;
};

#endif //IOBJECT_INCLUDED
