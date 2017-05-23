#pragma once
#include <string>
struct IObjectInfo
{
    virtual unsigned int getInterfaceId() const = 0;
    virtual std::string getInterfaceName() const = 0;
    // This is what actually gets displayed
    virtual std::string getObjectName() const = 0;
    virtual std::string getObjectTooltip() const = 0;
    virtual std::string getObjectHelp() const = 0;
    virtual std::string print() const = 0;
};
