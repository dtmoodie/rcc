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

#pragma once

#ifndef OBJECTINTERFACEPERMODULE_INCLUDED
#define OBJECTINTERFACEPERMODULE_INCLUDED
#include "ObjectInterface.h"
#include "RuntimeInclude.h"
#include "RuntimeLinkLibrary.h"
#include "RuntimeSourceDependency.h"
#include <string>
#include <vector>
#include <assert.h>
#include <RuntimeObjectSystem/IObjectInfo.h>
#include "RuntimeMacros.hpp"
#include "IObjectState.hpp"
#include "shared_ptr.hpp"

#ifndef RCCPPOFF
    #define AU_ASSERT( statement )  do { if (!(statement)) { volatile int* p = 0; int a = *p; if(a) {} } } while(0)
#else
    #define AU_ASSERT( statement ) assert( statement )
#endif //RCCPPOFF

// ****************************************************************************************
//                                 PerModuleInterface
// ****************************************************************************************
class PerModuleInterface : public IPerModuleInterface
{
public:
    static PerModuleInterface*  GetInstance();
    SystemTable*                m_pSystemTable = nullptr;

    void AddConstructor( IObjectConstructor* pConstructor );

    virtual std::vector<IObjectConstructor*>& GetConstructors();
    virtual void SetProjectIdForAllConstructors( unsigned short projectId_ );
    virtual void SetSystemTable( SystemTable* pSystemTable );

    SystemTable* GetSystemTable()
    {
        return m_pSystemTable;
    }

    virtual const std::vector<const char*>& GetRequiredSourceFiles() const;
    virtual void AddRequiredSourceFiles( const char* file_ );
    virtual void SetModuleFileName( const char* name )
    {
        m_ModuleFilename = name;
    }
    const char* GetCompiledPath() const
    {
#ifdef COMPILE_PATH
        return COMPILE_PATH;
#else
        return "";
#endif
    }

    virtual const char* GetModuleFileName() const
    {
        return m_ModuleFilename.c_str();
    }

    virtual void AddInterface(const std::string& name, unsigned int iid,
                              bool(*inheritance_f)(unsigned int),
                              bool(*direct_inheritance_f)(unsigned int))
    {
        InterfaceInfo info;
        info.iid = iid;
        info.inheritance_f = inheritance_f;
        info.direct_inheritance_f = direct_inheritance_f;
        info.name = name;
        m_interface_info.emplace_back(std::move(info));
    }

    virtual std::vector<InterfaceInfo> GetInterfaces() const
    {
        return m_interface_info;
    }

private:
    PerModuleInterface();

    ~PerModuleInterface()
    {
    }


    static PerModuleInterface*          ms_pObjectManager;
    std::vector<IObjectConstructor*>    m_ObjectConstructors;
    std::vector<const char*>            m_RequiredSourceFiles;
    std::string                         m_ModuleFilename;
    std::vector<InterfaceInfo>  m_interface_info;
};


// ****************************************************************************************
//                                 Concrete constructor
// ****************************************************************************************

template<typename T> class TObjectConstructorConcrete: public IObjectConstructor
{
public:
    TObjectConstructorConcrete(
#ifndef RCCPPOFF
        const char* Filename,
        IRuntimeIncludeFileList*        pIncludeFileList_,
        IRuntimeSourceDependencyList*   pSourceDependencyList_,
        IRuntimeLinkLibraryList*        pLinkLibraryList,
#endif
        bool                            bIsSingleton,
        bool                            bIsAutoConstructSingleton,
        IObjectInfo*                    pObjectInfo = NULL)
        : m_bIsSingleton(               bIsSingleton )
        , m_bIsAutoConstructSingleton(  bIsAutoConstructSingleton )
        , m_pModuleInterface(0)
        , m_Project(0)
#ifndef RCCPPOFF
        , m_FileName(                   Filename )
        , m_pIncludeFileList(pIncludeFileList_)
        , m_pSourceDependencyList(pSourceDependencyList_)
        , m_pLinkLibraryList(pLinkLibraryList)
        , m_pObjectInfo(pObjectInfo)
#endif
    {
#ifndef RCCPPOFF
        // add path to filename
        #ifdef COMPILE_PATH
            m_FileName = COMPILE_PATH + m_FileName;
        #endif
#endif
        PerModuleInterface::GetInstance()->AddConstructor( this );
        m_pModuleInterface = PerModuleInterface::GetInstance();
        m_Id = InvalidId;
    }

    virtual IObject* Construct()
    {
        T* pT = 0;
        if( m_bIsSingleton && m_ConstructedObjects.size() && m_ConstructedObjects[0] )
        {
            if(m_ConstructedObjects[0])
            {
                return m_ConstructedObjects[0]->GetIObject();
            }
        }

        if( m_FreeIds.empty() )
        {
            PerTypeObjectId id = m_ConstructedObjects.size();

            pT = new T();
            pT->SetPerTypeId( id );
            m_ConstructedObjects.push_back( new IObjectSharedState(pT, this) );
        }
        else
        {
            PerTypeObjectId id = m_FreeIds.back();
            m_FreeIds.pop_back();
            pT = new T();
            pT->SetPerTypeId( id );
            if(m_ConstructedObjects[id])
            {
                AU_ASSERT(0 == m_ConstructedObjects[id]->GetIObject());
                m_ConstructedObjects[ id ]->object = pT;
            }else
            {
                m_ConstructedObjects[ id ] = new IObjectSharedState(pT, this);
            }
        }
        return pT;
    }

    virtual IObject* Construct(IObjectSharedState* state)
    {
        T* pT = 0;
        if( m_bIsSingleton && m_ConstructedObjects.size() && m_ConstructedObjects[0] )
        {
            if(m_ConstructedObjects[0])
            {
                return m_ConstructedObjects[0]->GetIObject();
            }
        }

        if( m_FreeIds.empty() )
        {
            PerTypeObjectId id = m_ConstructedObjects.size();

            pT = new T();
            pT->SetPerTypeId( id );
            state->SetObject(pT);
            state->SetConstructor(this);
            m_ConstructedObjects.push_back( state );
        }
        else
        {
            PerTypeObjectId id = m_FreeIds.back();
            m_FreeIds.pop_back();
            pT = new T();
            pT->SetPerTypeId( id );
            if(m_ConstructedObjects[id])
            {
                AU_ASSERT(0 == m_ConstructedObjects[id]->GetIObject());
                m_ConstructedObjects[ id ]->object = pT;
            }else
            {
                state->SetObject(pT);
                state->SetConstructor(this);
                m_ConstructedObjects[ id ] = state;
            }
        }
        return pT;
    }

    virtual void ConstructNull()
    {
        // should not occur for singletons
        AU_ASSERT( !m_bIsSingleton );
        m_ConstructedObjects.push_back( NULL );
    }

    virtual const char* GetName()
    {
        return T::GetTypeNameStatic();
    }

    virtual void SetProjectId( unsigned short projectId_ )
    {
        m_Project = projectId_;
    }

    virtual unsigned short GetProjectId() const
    {
        return m_Project;
    }
    virtual IObjectInfo* GetObjectInfo() const
    {
        return m_pObjectInfo;
    }

    virtual const char* GetFileName()
    {
#ifndef RCCPPOFF
        return m_FileName.c_str();
#else
        return 0;
#endif
    }

    virtual const char* GetCompiledPath()
    {
#ifndef RCCPPOFF
         #ifdef COMPILE_PATH
            return COMPILE_PATH;
        #else
            return "";
        #endif
#else
        return 0;
#endif
   }

    virtual const char* GetIncludeFile( size_t Num_ ) const
    {
#ifndef RCCPPOFF
        if( m_pIncludeFileList )
        {
            return m_pIncludeFileList->GetIncludeFile( Num_ );
        }
#endif
        return 0;
    }

    virtual size_t GetMaxNumIncludeFiles() const
    {
#ifndef RCCPPOFF
        if( m_pIncludeFileList )
        {
            return m_pIncludeFileList->MaxNum;
        }
#endif
        return 0;
    }

    virtual const char* GetLinkLibrary( size_t Num_ ) const
    {
#ifndef RCCPPOFF
        if( m_pLinkLibraryList )
        {
            return m_pLinkLibraryList->GetLinkLibrary( Num_ );
        }
#endif
        return 0;
    }

    virtual size_t GetMaxNumLinkLibraries() const
    {
#ifndef RCCPPOFF
        if( m_pLinkLibraryList )
        {
            return m_pLinkLibraryList->MaxNum;
        }
#endif
        return 0;
    }

    virtual SourceDependencyInfo GetSourceDependency( size_t Num_ ) const
    {
#ifndef RCCPPOFF
        if( m_pSourceDependencyList )
        {
            return m_pSourceDependencyList->GetSourceDependency( Num_ );
        }
#endif
        return SourceDependencyInfo::GetNULL();
    }

    virtual size_t GetMaxNumSourceDependencies() const
    {
#ifndef RCCPPOFF
        if( m_pSourceDependencyList )
        {
            return m_pSourceDependencyList->MaxNum;
        }
#endif
        return 0;
    }

    virtual bool GetIsSingleton() const
    {
        return m_bIsSingleton;
    }
    virtual bool        GetIsAutoConstructSingleton() const
    {
        return m_bIsSingleton && m_bIsAutoConstructSingleton;
    }


    virtual IObject* GetConstructedObject( PerTypeObjectId id ) const
    {
        if( m_ConstructedObjects.size() > id )
        {
            if(m_ConstructedObjects[id])
                return m_ConstructedObjects[id]->GetIObject();
        }
        return 0;
    }
    virtual IObjectSharedState*  GetState( PerTypeObjectId num ) const
    {
        if( m_ConstructedObjects.size() > num )
        {
            return m_ConstructedObjects[num];
        }
        return 0;
    }
    virtual size_t     GetNumberConstructedObjects() const
    {
        size_t count = 0;
        for(auto& obj : m_ConstructedObjects)
        {
            if(obj->GetIObject())
            {
                ++count;
            }
        }
        return count;
    }
    virtual ConstructorId GetConstructorId() const
    {
        return m_Id;
    }
    virtual void SetConstructorId( ConstructorId id )
    {
        if( InvalidId == m_Id )
        {
            m_Id = id;
        }
    }

    void DeRegister( PerTypeObjectId id )
    {
        //remove from constructed objects.
        //use swap with last one
        if( m_ConstructedObjects.size() - 1 == id )
        {
            //it's the last one, just remove it.
            m_ConstructedObjects.pop_back();
        }
        else
        {
            m_FreeIds.push_back( id );
            m_ConstructedObjects[ id ] = 0;
        }
    }
    virtual void ClearIfAllDeleted()
    {
        m_FreeIds.clear();
        m_ConstructedObjects.clear();
    }
    virtual uint32_t GetInterfaceId() const
    {
        return T::getHash();
    }
    std::string GetInterfaceName() const
    {
        return T::GetInterfaceName();
    }
    virtual const IPerModuleInterface*  GetPerModuleInterface() const
    {
        return m_pModuleInterface;
    }
private:
    bool                            m_bIsSingleton;
    bool                            m_bIsAutoConstructSingleton;
    std::vector<IObjectSharedState*>                 m_ConstructedObjects;
    std::vector<PerTypeObjectId>    m_FreeIds;
    ConstructorId                   m_Id;
    PerModuleInterface*             m_pModuleInterface;
    unsigned short                  m_Project;
    IObjectInfo*                    m_pObjectInfo;
#ifndef RCCPPOFF
    std::string                     m_FileName;
    IRuntimeIncludeFileList*        m_pIncludeFileList;
    IRuntimeSourceDependencyList*   m_pSourceDependencyList;
    IRuntimeLinkLibraryList*        m_pLinkLibraryList;
#endif
};

// ****************************************************************************************
//                                 Concrete class
// ****************************************************************************************
template<typename T> class TActual: public T::template  InterfaceHelper<T>
{
public:
    typedef T BASE_CLASS;
    // overload new/delete to get alignment correct
#ifdef _WIN32
    void* operator new(size_t size)
    {
        size_t align = __alignof( TActual<T> );
        return _aligned_malloc( size, align );
    }
    void operator delete(void* p)
    {
        _aligned_free( p );
    }
#else
    void* operator new(size_t size)
    {
        size_t align = __alignof__( TActual<T> );
        void* pRet;
        int retval = posix_memalign( &pRet, align, size );
        (void)retval;    //unused
        return pRet;
    }
    void operator delete(void* p)
    {
        free( p );
    }
#endif //_WIN32
    friend class TObjectConstructorConcrete<TActual>;
    virtual ~TActual()
    {
        if(!T::IsRuntimeDelete())
        {
            if(auto state = m_Constructor.GetState(m_Id))
                state->SetObject(nullptr);
        }
    }
    virtual PerTypeObjectId GetPerTypeId() const { return m_Id; }
    virtual IObjectConstructor* GetConstructor() const { return &m_Constructor; }
    static IObjectConstructor* GetConstructorStatic(){ return &m_Constructor; }
    static const char* GetTypeNameStatic();
    virtual const char* GetTypeName() const
    {
        return GetTypeNameStatic();
    }
private:
    void SetPerTypeId( PerTypeObjectId id ) { m_Id = id; }
    PerTypeObjectId m_Id;
    static TObjectConstructorConcrete<TActual> m_Constructor;
};
// ****************************************************************************************
//                                 Registration Macros
// ****************************************************************************************

#ifndef RCCPPOFF
    #define REGISTERBASE( T, bIsSingleton, bIsAutoConstructSingleton, pObjectInfo )    \
        static RuntimeIncludeFiles< __COUNTER__ >       g_includeFileList_##T; \
        static RuntimeSourceDependency< __COUNTER__ >   g_sourceDependencyList_##T; \
        static RuntimeLinkLibrary< __COUNTER__ >        g_linkLibraryList_##T; \
    template<> TObjectConstructorConcrete< TActual< T > > TActual< T >::m_Constructor( __FILE__, &g_includeFileList_##T, &g_sourceDependencyList_##T, &g_linkLibraryList_##T, bIsSingleton, bIsAutoConstructSingleton, pObjectInfo);\
    template<> const char* TActual< T >::GetTypeNameStatic() { return #T; } \
    template class TActual< T >;
#else
    #define REGISTERBASE( T, bIsSingleton, bIsAutoConstructSingleton, pObjectInfo )    \
    template<> TObjectConstructorConcrete< TActual< T > > TActual< T >::m_Constructor( bIsSingleton, bIsAutoConstructSingleton, pObjectInfo); \
    template<> const char* TActual< T >::GetTypeNameStatic() { return #T; } \
    template class TActual< T >;
#endif

//NOTE: the file macro will only emit the full path if /FC option is used in visual studio or /ZI (Which forces /FC)
#define REGISTERCLASS_1( T )                REGISTERBASE( T, false, false, NULL )
#define REGISTERCLASS_2( T, pObjectInfo)    REGISTERBASE( T, false, false, pObjectInfo )

#define REGISTERSINGLETON_2( T, bIsAutoConstructSingleton )                    REGISTERBASE( T, true, bIsAutoConstructSingleton, NULL )
#define REGISTERSINGLETON_3( T, bIsAutoConstructSingleton, pObjectInfo )    REGISTERBASE( T, true, bIsAutoConstructSingleton, pObjectInfo )

#ifdef _MSC_VER
#define REGISTERCLASS(...) RCC_PP_CAT(RCC_PP_OVERLOAD( REGISTERCLASS_, __VA_ARGS__ )(__VA_ARGS__), RCC_PP_EMPTY())

#define REGISTERSINGLETON(...) RCC_PP_CAT(RCC_PP_OVERLOAD( REGISTERSINGLETON_, __VA_ARGS__ )(__VA_ARGS__), RCC_PP_EMPTY())
#else
#define REGISTERCLASS(...) RCC_PP_OVERLOAD( REGISTERCLASS_, __VA_ARGS__ )(__VA_ARGS__)

#define REGISTERSINGLETON(...) RCC_PP_OVERLOAD( REGISTERSINGLETON_, __VA_ARGS__ )(__VA_ARGS__)
#endif

#endif // OBJECTINTERFACEPERMODULE_INCLUDED
