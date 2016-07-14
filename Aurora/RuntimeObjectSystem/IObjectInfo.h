#pragma once
#include <string>
struct IObjectInfo
{
    virtual int GetInterfaceId() = 0;
    // This is what actually gets displayed
    virtual std::string GetObjectName() = 0;
    virtual std::string GetObjectTooltip() = 0;
    virtual std::string GetObjectHelp() = 0;
    virtual std::string Print() = 0;
};