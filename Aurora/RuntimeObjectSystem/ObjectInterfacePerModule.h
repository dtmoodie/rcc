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

#ifndef OBJECTINTERFACEPERMODULE_INCLUDED
#define OBJECTINTERFACEPERMODULE_INCLUDED

#include "ObjectInterface.h"
#include "RuntimeInclude.h"
#include "RuntimeLinkLibrary.h"
#include "RuntimeMacros.hpp"
#include "RuntimeSourceDependency.h"
#include <RuntimeObjectSystem/IObjectInfo.h>

#include <assert.h>
#include <string>
#include <vector>

#ifndef RCCPPOFF
    #define AU_ASSERT( statement )  do { if (!(statement)) { volatile int* p = nullptr; int a = *p; if(a) {} } } while(0)
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

    virtual std::vector<IObjectConstructor*>& GetConstructors() override;
    virtual void SetProjectIdForAllConstructors( unsigned short projectId_ ) override;
    virtual void SetSystemTable( SystemTable* pSystemTable ) override;

    SystemTable* GetSystemTable();

    virtual const std::vector<const char*>& GetRequiredSourceFiles() const override;
    virtual void AddRequiredSourceFiles( const char* file_ ) override;

    virtual void AddDelayInitFunction(const std::function<void(SystemTable*)>&) override;
    virtual void SetModuleFileName( const char* name ) override;

    const char* GetCompiledPath() const
    {
#ifdef COMPILE_PATH
        return COMPILE_PATH;
#else
        return "";
#endif
    }

    virtual const char* GetModuleFileName() const override;

    virtual void AddInterface(const std::string& name, unsigned int iid,
                              bool(*inheritance_f)(unsigned int),
                              bool(*direct_inheritance_f)(unsigned int)) override;

    virtual std::vector<InterfaceInfo> GetInterfaces() const override;

private:
    PerModuleInterface();

    ~PerModuleInterface() override;

    static PerModuleInterface*          ms_pObjectManager;
    std::vector<IObjectConstructor*>    m_ObjectConstructors;
    std::vector<const char*>            m_RequiredSourceFiles;
    std::string                         m_ModuleFilename;
    std::vector<InterfaceInfo>  m_interface_info;
    using DelayFunc_t = std::function<void(SystemTable*)>;
    std::vector<DelayFunc_t> m_DelayInitFuncs;
};


// ****************************************************************************************
//                                 Concrete constructor
// ****************************************************************************************

template<typename T> 
class TObjectConstructorConcrete: public IObjectConstructor
{
public:
    TObjectConstructorConcrete(
#ifndef RCCPPOFF
        const char* Filename,
        const IRuntimeIncludeFileList*        pIncludeFileList_,
        const IRuntimeSourceDependencyList*   pSourceDependencyList_,
        const IRuntimeLinkLibraryList*        pLinkLibraryList,
#endif
        bool                            bIsSingleton,
        bool                            bIsAutoConstructSingleton,
        const IObjectInfo*              pObjectInfo = nullptr)
        : m_bIsSingleton(               bIsSingleton )
        , m_bIsAutoConstructSingleton(  bIsAutoConstructSingleton )
        , m_pModuleInterface(nullptr)
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

    rcc::shared_ptr<IObject> Construct() override
    {
        if( m_bIsSingleton && !m_ConstructedObjects.empty() )
        {
            auto obj = m_ConstructedObjects[0].lock();
            if(obj)
            {
                return {obj};
            }
        }
        T* pT = nullptr;
        rcc::shared_ptr<IObject> out;
        if( m_FreeIds.empty() )
        {
            PerTypeObjectId id = m_ConstructedObjects.size();

            pT = new T();
            pT->SetPerTypeId( id );
            auto control_block = std::make_shared<TObjectControlBlock<typename T::BASE_CLASS>>(pT);
            out = rcc::shared_ptr<IObject>( control_block );
            m_ConstructedObjects.push_back( control_block );
        }
        else
        {
            PerTypeObjectId id = m_FreeIds.back();
            m_FreeIds.pop_back();
            pT = new T();
            pT->SetPerTypeId( id );
            auto control_block = m_ConstructedObjects[ id ].lock();
            if(control_block)
            {
                AU_ASSERT(nullptr == control_block->GetObject());
                control_block->SetObject(pT);
                m_ConstructedObjects[ id ] = control_block;
            }else
            {
                control_block = std::make_shared<TObjectControlBlock<typename T::BASE_CLASS>>(pT);
                m_ConstructedObjects[ id ] = control_block;
            }
            out = rcc::shared_ptr<IObject>(control_block);
        }
        return out;
    }

    // hopefully since we're now using shared ptrs to control blocks, we can get rid of this overload soon
    rcc::shared_ptr<IObject> Construct(std::shared_ptr<IObjectControlBlock> control_block) override
    {
        if( m_bIsSingleton && !m_ConstructedObjects.empty() )
        {
            auto obj = m_ConstructedObjects[0].lock();
            if(obj)
            {
                return {obj};
            }
        }

        T* pT = nullptr;

        if( m_FreeIds.empty() )
        {
            PerTypeObjectId id = m_ConstructedObjects.size();

            pT = new T();
            pT->SetPerTypeId( id );
            control_block->SetObject(pT);
            auto typed_control_block = std::dynamic_pointer_cast<TObjectControlBlock<typename T::BASE_CLASS>>(control_block);
            assert(typed_control_block != nullptr);
            assert(typed_control_block.use_count() > 2); // If the use count == 1, then this object is about to be deleted and no one cares for it
            m_ConstructedObjects.push_back( typed_control_block );
        }
        else
        {
            PerTypeObjectId id = m_FreeIds.back();
            m_FreeIds.pop_back();
            pT = new T();
            pT->SetPerTypeId( id );
            auto _control_block = m_ConstructedObjects[ id ].lock();
            if(_control_block)
            {
                AU_ASSERT(nullptr == _control_block->GetObject());
                _control_block->SetObject(pT);
                m_ConstructedObjects[ id ] = _control_block;
                control_block = _control_block;
            }else
            {
                control_block->SetObject(pT);
                auto typed_control_block = std::dynamic_pointer_cast<TObjectControlBlock<typename T::BASE_CLASS>>(control_block);
                m_ConstructedObjects[ id ] = typed_control_block;
            }
            control_block = _control_block;
        }
        return rcc::shared_ptr<IObject>(control_block);
    }

    void ConstructNull() override
    {
        // should not occur for singletons
        AU_ASSERT( !m_bIsSingleton );
        m_ConstructedObjects.push_back( {} );
    }

    const char* GetName() override
    {
        return T::GetTypeNameStatic();
    }

    void SetProjectId( unsigned short projectId_ ) override
    {
        m_Project = projectId_;
    }

    unsigned short GetProjectId() const override
    {
        return m_Project;
    }

    const IObjectInfo* GetObjectInfo() const override
    {
        return m_pObjectInfo;
    }

    const char* GetFileName() override
    {
#ifndef RCCPPOFF
        return m_FileName.c_str();
#else
        return nullptr;
#endif
    }

    const char* GetCompiledPath() override
    {
#ifndef RCCPPOFF
         #ifdef COMPILE_PATH
            return COMPILE_PATH;
        #else
            return "";
        #endif
#else
        return nullptr;
#endif
   }

    const char* GetIncludeFile( size_t Num_ ) const override
    {
#ifndef RCCPPOFF
        if( m_pIncludeFileList )
        {
            return m_pIncludeFileList->GetIncludeFile( Num_ );
        }
#endif
        return nullptr;
    }

    size_t GetMaxNumIncludeFiles() const override
    {
#ifndef RCCPPOFF
        if( m_pIncludeFileList )
        {
            return m_pIncludeFileList->MaxNum;
        }
#endif
        return 0;
    }

    const char* GetLinkLibrary( size_t Num_ ) const override
    {
#ifndef RCCPPOFF
        if( m_pLinkLibraryList )
        {
            return m_pLinkLibraryList->GetLinkLibrary( Num_ );
        }
#endif
        return nullptr;
    }

    size_t GetMaxNumLinkLibraries() const override
    {
#ifndef RCCPPOFF
        if( m_pLinkLibraryList )
        {
            return m_pLinkLibraryList->MaxNum;
        }
#endif
        return 0;
    }

    SourceDependencyInfo GetSourceDependency( size_t Num_ ) const override
    {
#ifndef RCCPPOFF
        if( m_pSourceDependencyList )
        {
            return m_pSourceDependencyList->GetSourceDependency( Num_ );
        }
#endif
        return SourceDependencyInfo::GetNULL();
    }

    size_t GetMaxNumSourceDependencies() const override
    {
#ifndef RCCPPOFF
        if( m_pSourceDependencyList )
        {
            return m_pSourceDependencyList->MaxNum;
        }
#endif
        return 0;
    }

    bool GetIsSingleton() const override
    {
        return m_bIsSingleton;
    }

    bool GetIsAutoConstructSingleton() const override
    {
        return m_bIsSingleton && m_bIsAutoConstructSingleton;
    }


    rcc::shared_ptr<IObject> GetConstructedObject( PerTypeObjectId id ) const override
    {
        if( m_ConstructedObjects.size() > id )
        {
            auto obj = m_ConstructedObjects[id].lock();
            if(obj)
                return {obj};
        }
        return {};
    }

    std::shared_ptr<IObjectControlBlock>  GetControlBlock( PerTypeObjectId num ) const override
    {
        if( m_ConstructedObjects.size() > num )
        {
            return m_ConstructedObjects[num].lock();
        }
        return {};
    }

    size_t     GetNumberConstructedObjects() const override
    {
        size_t count = 0;
        for(auto& control_block : m_ConstructedObjects)
        {
            auto locked = control_block.lock();
            if(locked && locked->GetObject())
            {
                ++count;
            }
        }
        return count;
    }

    ConstructorId GetConstructorId() const override
    {
        return m_Id;
    }

    void SetConstructorId( ConstructorId id ) override
    {
        if( InvalidId == m_Id )
        {
            m_Id = id;
        }
    }

    void ClearIfAllDeleted() override
    {
        m_FreeIds.clear();
        m_ConstructedObjects.clear();
    }

    uint32_t GetInterfaceId() const override
    {
        return T::getHash();
    }

    std::string GetInterfaceName() const override
    {
        return T::GetInterfaceName();
    }

    const IPerModuleInterface*  GetPerModuleInterface() const override
    {
        return m_pModuleInterface;
    }
private:
    bool                            m_bIsSingleton;
    bool                            m_bIsAutoConstructSingleton;
    std::vector<std::weak_ptr<TObjectControlBlock<typename T::BASE_CLASS>>>                 m_ConstructedObjects;
    std::vector<PerTypeObjectId>    m_FreeIds;
    ConstructorId                   m_Id;
    PerModuleInterface*             m_pModuleInterface;
    unsigned short                  m_Project;
    const IObjectInfo*              m_pObjectInfo;
#ifndef RCCPPOFF
    std::string                     m_FileName;
    const IRuntimeIncludeFileList*        m_pIncludeFileList;
    const IRuntimeSourceDependencyList*   m_pSourceDependencyList;
    const IRuntimeLinkLibraryList*        m_pLinkLibraryList;
#endif
};

// ****************************************************************************************
//                                 Concrete class
// ****************************************************************************************
template<typename T> 
class TActual: public T::template InterfaceHelper<T>
{
public:
    using BASE_CLASS = T;
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

#else // Unix platforms

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
            // I think this is no longer needed because we can get rid of the old control block?
            // I think this is no longer needed because the control block is what is deleting this object
            //if(auto control_block = m_Constructor.GetControlBlock(m_Id))    control_block->SetObject(nullptr);
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
