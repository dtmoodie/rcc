#include "IObject.h"


IObject::IObject() : _isRuntimeDelete(false)
{

}

IObject::~IObject()
{

}

void IObject::Serialize(ISimpleSerializer */*pSerializer*/)
{
}


void IObject::Init( bool /*isFirstInit*/ )
{

}

IObject* IObject::GetInterface(InterfaceID __iid)
{
    switch(__iid)
    {
    case s_interfaceID:
        return this;
    default:
        return nullptr;
    }
}


bool IObject::InheritsFrom(InterfaceID iid)
{
    if(iid == s_interfaceID)
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
