# How to include RCC in your own project
# 
#  # Get all include directories for this project and pass them to RCC.
#  # Alternatively you can only include the directories that you know you need in runtime compilable classes 
#  GET_PROPERTY(RCC_INCLUDE_DEPENDENCIES DIRECTORY PROPERTY INCLUDE_DIRECTORIES) 
#  
#  # Set the include directory for RCC to find your definition of an IObject
#  SET(IObject_INCLUDE ".....")
#  
#  # Semi-colon separated list of directories for searching for libraries
#  SET(RCC_LIBRARY_DIRS "..... linked library dirs ..........") 
#  Add this to your project to gain access to the object factory system
#  ADD_SUBDIRECTORY("RCC")
#  INCLUDE_DIRECTORIES(${RCC_INCLUDE_DIRS})
# 
#
# ================ C++ code example ====================================
# 
# #include "RuntimeObjectSystem.h"
#
# int main()
# {
#   RuntimeObjectSystem objectFactory;
#   objectFactory.Initialise(new CompileLogger(), nullptr);
#   auto constructor = objectFactory.GetObjectFactorySystem()->GetConstructor("MyObject");
#   MyObjectInterface* myObject = nullptr;
#   if(constructor)
#   {
#      IObject* obj = constructor->construct();
#      if(obj && obj->GetInterface(MyObjectInterfaceID))
#      {
#         myObject = static_cast<MyObjectInterface*>(obj);
#      }
#   }
#
#   boost::posix_time::ptime prevTime = boost::posix_time::microsec_clock::universal_time();
#	boost::posix_time::ptime currentTime = boost::posix_time::microsec_clock::universal_time();
#	boost::posix_time::time_duration delta = currentTime - prevTime;
#   while(true)
#   {
#      myObject->RunVirtualFunction();
#      prevTime = boost::posix_time::microsec_clock::universal_time();
#      currentTime = boost::posix_time::microsec_clock::universal_time();
#      delta = currentTime - prevTime;
#      if(delta.total_milliseconds() > 10)
#         continue;
#      prevTime = currentTime;
#    if( objectFactory.GetIsCompiledComplete())
#    {
#        objectFactory.LoadCompiledModule();
#    }
#    if(objectFactory.GetIsCompiling())
#    {
#       return true;
#    }else
#    {
#        objectFactory.GetFileChangeNotifier()->Update(float(delta.total_milliseconds())/1000.0);
#    }
#   }
#   return 0;
# }

cmake_minimum_required(VERSION 2.8)

project(RCC)

if(UNIX AND NOT APPLE)
	set(BUILD_TYPE SHARED)
else()
	set(BUILD_TYPE STATIC)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

include(Aurora/cmake/ProjectFilesExternal.cmake)
include_directories(${IObject_INCLUDE})

message(STATUS "IObject include dir: ${IObject_INCLUDE}")

add_library(RuntimeCompiler ${BUILD_TYPE} ${RuntimeCompiler_SRCS})
add_library(RuntimeObjectSystem ${BUILD_TYPE} ${RuntimeObjectSystem_SRCS})

target_link_libraries(RuntimeObjectSystem RuntimeCompiler)

if(UNIX)
	target_link_libraries(RuntimeObjectSystem dl)
endif()

SET(RCC_INCLUDE_DIRS "${IObject_INCLUDE};${CMAKE_CURRENT_SOURCE_DIR}/Aurora/RuntimeObjectSystem;${CMAKE_CURRENT_SOURCE_DIR}/Aurora/RuntimeCompiler" PARENT_SCOPE)

LIST(REMOVE_DUPLICATES RCC_INCLUDE_DEPENDENCIES) 
STRING(REGEX REPLACE ";" "+" RCC_INCLUDE_DEPENDENCIES "${RCC_INCLUDE_DEPENDENCIES}" )
STRING(REGEX REPLACE ";" "+" RCC_LIBRARY_DIRS "${RCC_LIBRARY_DIRS}" )

ADD_DEFINITIONS( -DRCC_INCLUDES="${RCC_INCLUDE_DEPENDENCIES}" )
ADD_DEFINITIONS( -DRCC_LIBRARY_DIRS="${RCC_LIBRARY_DIRS}" )
