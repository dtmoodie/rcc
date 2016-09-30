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
#    boost::posix_time::ptime currentTime = boost::posix_time::microsec_clock::universal_time();
#    boost::posix_time::time_duration delta = currentTime - prevTime;
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
    string(TOLOWER ${CMAKE_BUILD_TYPE} cmake_build_type_)
    if(cmake_build_type_ EQUAL "debug")
        add_definitions(-D_DEBUG)
    endif(cmake_build_type_ EQUAL "debug")
endif()

include(Aurora/cmake/ProjectFilesExternal.cmake)
include_directories(${IObject_INCLUDE})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Aurora/RuntimeCompiler)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Aurora/RuntimeObjectSystem)

FIND_PACKAGE(CUDA)
message(STATUS 
"RCC config:
  IObject include dir: ${IObject_INCLUDE}
  NVCC: ${CUDA_TOOLKIT_ROOT_DIR}
"
)
set(NVCC_COMPILER "")
IF(CUDA_FOUND)
    set(NVCC_COMPILER "${CUDA_NVCC_EXECUTABLE}")
ENDIF()
set(COMPILER_PATH ${CMAKE_CXX_COMPILER};${NVCC_COMPILER})


SET(RCC_INCLUDE_DIRS "${IObject_INCLUDE};${CMAKE_CURRENT_SOURCE_DIR}/Aurora/RuntimeObjectSystem;${CMAKE_CURRENT_SOURCE_DIR}/Aurora/RuntimeCompiler" PARENT_SCOPE)
if(RCC_INCLUDE_DEPENDENCIES)
  LIST(REMOVE_DUPLICATES RCC_INCLUDE_DEPENDENCIES) 
endif()
if(RCC_LIBRARY_DIRS_RELEASE)
  LIST(REMOVE_DUPLICATES RCC_LIBRARY_DIRS_RELEASE)
endif()
if(RCC_LIBRARY_DIRS_DEBUG)
  LIST(REMOVE_DUPLICATES RCC_LIBRARY_DIRS_DEBUG)
endif()
if(RCC_LIBRARY_DIRS_RELWITHDEBINFO)
  LIST(REMOVE_DUPLICATES RCC_LIBRARY_DIRS_RELWITHDEBINFO)
endif()

IF(WIN32)
    FILE(WRITE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/RCC_config.txt" "0\n${RCC_INCLUDE_DEPENDENCIES}\n${RCC_LIBRARY_DIRS_DEBUG};\n/FS ${RCC_COMPILE_FLAGS}")
    FILE(WRITE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/RCC_config.txt" "0\n${RCC_INCLUDE_DEPENDENCIES}\n\n${RCC_LIBRARY_DIRS_RELEASE};\n/FS ${RCC_COMPILE_FLAGS}")
    FILE(WRITE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/RelWithDebInfo/RCC_config.txt" "0\n${RCC_INCLUDE_DEPENDENCIES}\n\n${RCC_LIBRARY_DIRS_RELWITHDEBINFO};\n/FS ${RCC_COMPILE_FLAGS}")
    FILE(WRITE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/RCC_config.txt" "0\n${RCC_INCLUDE_DEPENDENCIES}\n${RCC_LIBRARY_DIRS_DEBUG}\n${RCC_LIBRARY_DIRS_RELWITHDEBINFO};\n/FS ${RCC_COMPILE_FLAGS}")
ELSE()
    FILE(WRITE "${CMAKE_BINARY_DIR}/RCC_config.txt" "0\n${RCC_INCLUDE_DEPENDENCIES}\n${RCC_LIBRARY_DIRS_DEBUG};${CMAKE_BINARY_DIR}\n${RCC_LIBRARY_DIRS_RELEASE};${CMAKE_BINARY_DIR}\n${RCC_COMPILE_FLAGS}\n${COMPILER_PATH}")
    FILE(WRITE "${CMAKE_CURRENT_BINARY_DIR}/RCC_config.txt" "0\n${RCC_INCLUDE_DEPENDENCIES}\n${RCC_LIBRARY_DIRS_DEBUG};${CMAKE_BINARY_DIR}\n${RCC_LIBRARY_DIRS_RELEASE};${CMAKE_BINARY_DIR}\n${RCC_COMPILE_FLAGS}\n${COMPILER_PATH}")
ENDIF()


add_library(RuntimeCompiler ${BUILD_TYPE} ${RuntimeCompiler_SRCS})
add_library(RuntimeObjectSystem ${BUILD_TYPE} ${RuntimeObjectSystem_SRCS})
target_link_libraries(RuntimeObjectSystem RuntimeCompiler)

install(TARGETS RuntimeObjectSystem RuntimeCompiler
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

install(DIRECTORY Aurora/include
        DESTINATION include
        FILES_MATCHING PATTERN "*.h")
install(DIRECTORY Aurora/include
        DESTINATION include
        FILES_MATCHING PATTERN "*.hpp")

if(UNIX)
        target_link_libraries(RuntimeObjectSystem dl)
endif()
