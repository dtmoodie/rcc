#pragma once
#include <string>
class IObjectConstructor;
struct IObjectInfo
{
    virtual unsigned int GetInterfaceId() const = 0;
    virtual std::string GetInterfaceName() const = 0;
    // This is what actually gets displayed
    virtual std::string GetObjectName() const = 0;
    virtual std::string Print() const = 0;
    virtual IObjectConstructor* GetConstructor() const = 0;
};
