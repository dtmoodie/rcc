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

#ifndef OBJECTINTERFACE_INCLUDED
#define OBJECTINTERFACE_INCLUDED
#include "IObject.h"
#include "shared_ptr.hpp"

#include <functional>
#include <memory>
#include <stdlib.h>
#include <string>
#include <vector>

// more msvc macro stuffs -_-
#ifdef GetModuleFileName
#undef GetModuleFileName
#endif


struct SystemTable; //This is the interface to your own engine code, which you need to define yourself if required.
struct IObject;
struct IObjectInfo;
struct SourceDependencyInfo;
struct IObjectSharedState;
struct IPerModuleInterface;
struct IObjectControlBlock;

struct IObjectConstructor
{
    virtual                      ~IObjectConstructor() {}
    virtual rcc::shared_ptr<IObject>  Construct() = 0;
    virtual rcc::shared_ptr<IObject>  Construct(std::shared_ptr<IObjectControlBlock> control_block) = 0;
    virtual void                 ConstructNull() = 0;    //for use in object replacement, ensures a deleted object can be replaced
    virtual const char*          GetName() = 0;
    virtual const char*          GetFileName() = 0;
    virtual const char*          GetCompiledPath() = 0;
    virtual size_t               GetMaxNumIncludeFiles() const = 0;
    virtual const char*          GetIncludeFile( size_t Num_ ) const = 0;
    virtual size_t               GetMaxNumLinkLibraries() const = 0;
    virtual const char*          GetLinkLibrary( size_t Num_ ) const = 0;
    virtual size_t               GetMaxNumSourceDependencies() const = 0;
    virtual SourceDependencyInfo GetSourceDependency( size_t Num_ ) const = 0;
    virtual void                 SetProjectId( unsigned short projectId_ ) = 0;
    virtual unsigned short       GetProjectId() const = 0;
    virtual const IObjectInfo*   GetObjectInfo() const = 0;
    virtual const IPerModuleInterface*  GetPerModuleInterface() const = 0;

    // Singleton functions
    virtual bool                 GetIsSingleton() const = 0;
    virtual bool                 GetIsAutoConstructSingleton() const = 0;
    rcc::shared_ptr<IObject>     GetSingleton()
    {
        return Construct();
    }

    virtual rcc::shared_ptr<IObject>  GetConstructedObject( PerTypeObjectId num ) const = 0;    //should return 0 for last or deleted object
    virtual std::shared_ptr<IObjectControlBlock>  GetControlBlock( PerTypeObjectId num ) const = 0;
    virtual size_t               GetNumberConstructedObjects() const = 0;
    virtual ConstructorId        GetConstructorId() const = 0;
    virtual void                 SetConstructorId( ConstructorId id ) = 0;                    //take care how you use this - should only be used by id service
    virtual void                 ClearIfAllDeleted() = 0;                                    //if there are no objects left then clear internal memory (does not reduce memory consumption)

    virtual uint32_t             GetInterfaceId() const = 0;
    virtual std::string          GetInterfaceName() const = 0;
};

struct IPerModuleInterface
{
    struct InterfaceInfo{
        std::string name;
        unsigned int iid;
        bool(*inheritance_f)(unsigned int);
        bool(*direct_inheritance_f)(unsigned int);
    };
    virtual std::vector<IObjectConstructor*>& GetConstructors() = 0;
    virtual void SetProjectIdForAllConstructors( unsigned short projectId_ ) = 0;
    virtual void SetSystemTable( SystemTable* pSystemTable ) = 0;
    virtual const std::vector<const char*>& GetRequiredSourceFiles() const = 0;
    virtual void AddRequiredSourceFiles( const char* file_ ) = 0;
    virtual void AddDelayInitFunction(const std::function<void(SystemTable*)>&) = 0;
    virtual void SetModuleFileName( const char* name ) = 0;
    virtual const char* GetModuleFileName() const = 0;
    virtual void AddInterface(const std::string& name, unsigned int iid,
                              bool(*inheritance_f)(unsigned int),
                              bool(*direct_inheritance_f)(unsigned int)) = 0;
    virtual std::vector<InterfaceInfo> GetInterfaces() const = 0;
    virtual void SetConstructorAddedCallback(std::function<void(IObjectConstructor*)>) = 0;
    virtual ~IPerModuleInterface() {}
};

#ifdef _WIN32
typedef IPerModuleInterface* (__cdecl *GETPerModuleInterface_PROC)(void);
#else
typedef IPerModuleInterface* ( *GETPerModuleInterface_PROC)(void);
#endif

#include "IObjectControlBlock.hpp"


#endif //OBJECTINTERFACE_INCLUDED
