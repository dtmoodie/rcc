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
CMAKE_POLICY(SET CMP0048 NEW)
project(RCC VERSION 1.0.0)
set(CMAKE_DEBUG_POSTFIX "d")
SET(RCC_VERBOSE_CONFIG FALSE CACHE BOOL "Set to true to output verbose debugging information in RCC macros")

# Get all propreties that cmake supports
execute_process(COMMAND cmake --help-property-list OUTPUT_VARIABLE CMAKE_PROPERTY_LIST)

# Convert command output into a CMake list
STRING(REGEX REPLACE ";" "\\\\;" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")
STRING(REGEX REPLACE "\n" ";" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")
STRING(REPLACE "LOCATION" "" CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}")
set(CMAKE_PROPERTY_LIST "${CMAKE_PROPERTY_LIST}" CACHE INTERNAL "")
function(print_properties)
    message ("CMAKE_PROPERTY_LIST = ${CMAKE_PROPERTY_LIST}")
endfunction(print_properties)

function(print_property prop tgt)
    string(REPLACE "<CONFIG>" "${CMAKE_BUILD_TYPE}" prop ${prop})
    get_property(propval TARGET ${tgt} PROPERTY ${prop} SET)
    if (propval)
        get_target_property(propval ${tgt} ${prop})
        message ("${tgt} ${prop} = ${propval}")
    endif()
endfunction()

function(print_target_properties tgt)
    if(NOT TARGET ${tgt})
      message("There is no target named '${tgt}'")
      return()
    endif()

    foreach (prop ${CMAKE_PROPERTY_LIST})
        print_property(${prop} ${tgt})
    endforeach(prop)
endfunction(print_target_properties)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(cmake/rcc_find_path.cmake)
include(cmake/rcc_find_library.cmake)
include(cmake/rcc_link_lib.cmake)
include(cmake/rcc_strip_extension.cmake)
include(cmake/rcc_config.cmake)

cmake_policy(SET CMP0054 NEW)


if(UNIX AND NOT APPLE)
    set(BUILD_TYPE STATIC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
else()
    set(BUILD_TYPE STATIC)
endif()

include(Aurora/cmake/ProjectFilesExternal.cmake)

FIND_PACKAGE(CUDA QUIET)
message(STATUS
"RCC config:
  IObject include dir: ${IObject_INCLUDE}
  NVCC: ${CUDA_TOOLKIT_ROOT_DIR}"
)

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

#WRITE_RCC_CONFIG("${RCC_INCLUDE_DEPENDENCIES}" "${RCC_LIBARY_DIRS_DEBUG}" "${RCC_LIBRARY_DIRS_RELEASE}" "${RCC_COMPILE_FLAGS}")

add_library(RuntimeCompiler ${BUILD_TYPE} ${RuntimeCompiler_SRCS})

add_library(RuntimeObjectSystem STATIC ${RuntimeObjectSystem_SRCS})
target_link_libraries(RuntimeObjectSystem RuntimeCompiler)
set_property(TARGET RuntimeCompiler PROPERTY CXX_STANDARD 11)
set_property(TARGET RuntimeObjectSystem PROPERTY CXX_STANDARD 11)
if(NOT IObject_INCLUDE)
  set(IObject_INCLUDE Aurora/include)
endif(NOT IObject_INCLUDE)

target_include_directories(RuntimeCompiler
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Aurora>
)

target_include_directories(RuntimeCompiler
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Aurora/RuntimeObjectSystem;
    ${CMAKE_CURRENT_SOURCE_DIR}/Aurora/RuntimeCompiler>
)

target_include_directories(RuntimeObjectSystem
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Aurora>
)

target_include_directories(RuntimeObjectSystem
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/Aurora/RuntimeObjectSystem;
        ${CMAKE_CURRENT_SOURCE_DIR}/Aurora/RuntimeCompiler>
)
set(RCC_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/Aurora" CACHE PATH "")

if(NOT WIN32)
    target_compile_options(RuntimeCompiler PRIVATE -fPIC -fvisibility=hidden)
    target_compile_options(RuntimeObjectSystem PRIVATE -fPIC -fvisibility=hidden)
endif(NOT WIN32)

configure_file(RCCConfig.cmake.in
  "${PROJECT_BINARY_DIR}/RCCConfig.cmake" @ONLY)

export(TARGETS RuntimeCompiler RuntimeObjectSystem
      FILE "${PROJECT_BINARY_DIR}/RCCTargets.cmake"
)

file(COPY "cmake" DESTINATION ${PROJECT_BINARY_DIR})

export(PACKAGE RCC)

install(TARGETS RuntimeObjectSystem RuntimeCompiler
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)

install(TARGETS RuntimeObjectSystem RuntimeCompiler
        DESTINATION lib
        EXPORT RCCTargets
)

install(DIRECTORY Aurora/include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY Aurora/include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.hpp"
)

install(DIRECTORY Aurora/RuntimeObjectSystem
        DESTINATION include
        FILES_MATCHING PATTERN "*.hpp"
)

install(DIRECTORY Aurora/RuntimeCompiler
        DESTINATION include
        FILES_MATCHING PATTERN "*.hpp"
)

install(DIRECTORY Aurora/ct
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

install(DIRECTORY Aurora/RuntimeObjectSystem
        DESTINATION include
        FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY Aurora/RuntimeCompiler
        DESTINATION include
        FILES_MATCHING PATTERN "*.h"
)

if(WIN32)
    install(FILES ${CMAKE_BINARY_DIR}/RuntimeCompiler.dir/Debug/RuntimeCompiler.pdb
        DESTINATION lib COMPONENT dev RENAME RuntimeCompilerd.pdb
    )
    install(FILES ${CMAKE_BINARY_DIR}/RuntimeObjectSystem.dir/Debug/RuntimeObjectSystem.pdb
        DESTINATION lib COMPONENT dev RENAME RuntimeObjectSystemd.pdb
    )
    install(FILES ${CMAKE_BINARY_DIR}/RuntimeCompiler.dir/RelWithDebInfo/RuntimeCompiler.pdb
        DESTINATION lib COMPONENT dev
    )
    install(FILES ${CMAKE_BINARY_DIR}/RuntimeObjectSystem.dir/RelWithDebInfo/RuntimeObjectSystem.pdb
        DESTINATION lib COMPONENT dev
    )
endif(WIN32)

install(DIRECTORY cmake DESTINATION share/RCC FILES_MATCHING PATTERN "*.cmake")

install(FILES
  "${PROJECT_BINARY_DIR}/RCCConfig.cmake"
  DESTINATION "${CMAKE_INSTALL_PREFIX}/share/RCC" COMPONENT dev
)

install(EXPORT RCCTargets DESTINATION "${CMAKE_INSTALL_PREFIX}/share/RCC"
)

if(UNIX)
  target_link_libraries(RuntimeObjectSystem dl)
endif()
add_subdirectory("Aurora/tests")
