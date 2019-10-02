#include "IObject.h"
#include "ObjectInterface.h"

uint32_t IObject::getHash()
{
    return 1234;
}

const InterfaceID IObject::s_interfaceID = IObject::getHash();

IObject::IObject() : _isRuntimeDelete(false)
{

}

IObject::~IObject()
{

}

void IObject::Serialize(ISimpleSerializer* /*pSerializer*/)
{
}


void IObject::Init( bool /*isFirstInit*/ )
{

}

void* IObject::GetInterface(InterfaceID __iid)
{
    if(__iid == getHash())
    {
        return this;
    }
    return nullptr;
}


bool IObject::InheritsFrom(InterfaceID iid)
{
    if(iid == getHash())
    {
        return true;
    }
    return false;
}


bool IObject::DirectlyInheritsFrom(InterfaceID /*iid*/)
{
    return false;
}

std::string IObject::GetInterfaceName()
{
    return "IObject";
}

void IObject::GetObjectId( ObjectId& id ) const
{
    id.m_ConstructorId = GetConstructor()->GetConstructorId();
    id.m_PerTypeId = GetPerTypeId();
}

ObjectId IObject::GetObjectId() const
{
    ObjectId ret;
    GetObjectId( ret );
    return ret;
}


bool IObject::IsRuntimeDelete()
{
    return _isRuntimeDelete;
}
