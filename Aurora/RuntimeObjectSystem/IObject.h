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
#include "RuntimeObjectSystem/InterfaceDatabase.hpp"
#include "RuntimeObjectSystem/ObjectInterface.h"
#include <algorithm>
#include <assert.h>
#include <ct/Hash.hpp>
#include <ct/Object.hpp>
#include <iostream>

struct ISimpleSerializer;
class ObjectFactorySystem;

typedef unsigned int InterfaceID;
#ifdef _MSC_VER
#define INTERFACE_HASH static constexpr InterfaceID class_hash() {return hashClassName(__FUNCTION__);}
#else
#define INTERFACE_HASH static constexpr InterfaceID class_hash() {return hashClassName(__PRETTY_FUNCTION__);}
#endif


template<class TInterface>
struct RegisterInterface
{
    RegisterInterface()
    {
        rcc::InterfaceDatabase::RegisterInterface(TInterface::GetInterfaceName(),
                                                              TInterface::getHash(),
                                                              &TInterface::InheritsFrom,
                                                              &TInterface::DirectlyInheritsFrom);
    }
};

// Template to help with IIDs
template< typename TInferior, typename TSuper, size_t Version = 0>
struct TInterface : public TSuper
{
    TInterface()
    {
        (void)&s_register_interface;
    }
    static constexpr uint32_t getHash() { return ct::ctcrc32(__CT_STRUCT_MAGIC_FUNCTION__); }

    static const InterfaceID s_interfaceID
#ifndef __CUDACC__
        = getHash()
#endif
        ;

    static size_t GetInterfaceVersion(){
        return Version;
    }
    static size_t GetInterfaceAbiHash(){
        size_t seed = Version;
        seed ^= TSuper::GetInterfaceAbiHash() + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }

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

    static bool InheritsFrom(InterfaceID iid)
    {
#ifndef __CUDACC__
        if(iid == TInterface::getHash())
        {
            return true;
        }else
        {
            return TSuper::InheritsFrom(iid);
        }
#else
        return false;
#endif
    }

    static bool DirectlyInheritsFrom(InterfaceID iid)
    {
#ifndef __CUDACC__
        return iid == TSuper::getHash();
#else
        return false;
#endif
    }

    virtual IObject* GetInterface( InterfaceID _iid)
    {
#ifndef __CUDACC__
        if(_iid == getHash())
        {
            return this;
        }
        return TSuper::GetInterface(_iid);
#else
        return nullptr;
#endif
    }

private:
    static RegisterInterface<TInterface<TInferior, TSuper>> s_register_interface;

};

template<typename TInferior, typename TSuper, size_t Version>
RegisterInterface<TInterface<TInferior, TSuper>> TInterface<TInferior, TSuper, Version>::s_register_interface;

// IObject itself below is a special case as the base class
// Also it doesn't hurt to have it coded up explicitly for reference
struct IObject
{

    static uint32_t getHash() { return ct::ctcrc32(__CT_STRUCT_MAGIC_FUNCTION__); }
    static const InterfaceID s_interfaceID
#ifndef __CUDACC__
        = ct::ctcrc32("IObject")
#endif
        ;

    static bool InheritsFrom(InterfaceID id);

    virtual IObject* GetInterface(InterfaceID __iid);

    static size_t GetInterfaceAbiHash(){
        return 0;
    }

    template< typename T>
    void GetInterface( T** pReturn )
    {
#ifndef __CUDACC__
        GetInterface( T::getHash(), static_cast<void**>(pReturn) );
#endif
    }

    static bool DirectlyInheritsFrom(InterfaceID iid);

    static std::string GetInterfaceName();

    IObject();
    virtual ~IObject();

    // Perform any object initialization
    // Should be called with isFirstInit=true on object creation
    // Will automatically be called with isFirstInit=false whenever a system serialization is performed
    virtual void Init( bool isFirstInit );

    //return the PerTypeObjectId of this object, which is unique per class
    virtual PerTypeObjectId GetPerTypeId() const = 0;

    virtual void GetObjectId( ObjectId& id ) const;

    virtual ObjectId GetObjectId() const;

    //return the constructor for this class
    virtual IObjectConstructor* GetConstructor() const = 0;

    //serialise is not pure virtual as many objects do not need state
    virtual void Serialize(ISimpleSerializer *pSerializer);

    virtual const char* GetTypeName() const = 0;

protected:
    bool IsRuntimeDelete();

private:
    friend class ObjectFactorySystem;

    // Set to true when object is being deleted because a new version has been created
    // Destructor should use this information to not delete other IObjects in this case
    // since these objects will still be needed
    bool _isRuntimeDelete;
};

#endif //IOBJECT_INCLUDED
