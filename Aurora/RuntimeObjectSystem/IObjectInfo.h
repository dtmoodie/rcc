#pragma once
#include <string>
#include <RuntimeObjectSystem/shared_ptr.hpp>
struct IObjectConstructor;
typedef unsigned int InterfaceID;
struct IObjectInfo
{
    virtual ~IObjectInfo();
    /*!
     * \brief The Verbosity enum determines how verbose of a print statement to use
     */
    enum Verbosity{
        INFO = 0,  // General information about the object
        DEBUG = 1, // Debug information including the shared library it was loaded from and the path to source file
        RCC = 2    // compile information
    };

    virtual unsigned int GetInterfaceId() const = 0;
    virtual std::string GetInterfaceName() const = 0;
    // This is what actually gets displayed
    virtual std::string GetObjectName() const = 0;
    /*!
     * \brief Prints information about the object constructed by this constructor
     * \param verbosity level of returned string
     * \return human readable string description of object
     */
    virtual std::string Print(Verbosity verbosity = Verbosity::INFO) const = 0;
    virtual IObjectConstructor* GetConstructor() const = 0;
    virtual bool InheritsFrom(InterfaceID iid) const = 0;
};
