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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(cmake/rcc_find_path.cmake)
include(cmake/rcc_find_library.cmake)
include(cmake/rcc_link_lib.cmake)
include(cmake/rcc_config.cmake)

cmake_policy(SET CMP0054 NEW)


if(UNIX AND NOT APPLE)
    set(BUILD_TYPE STATIC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
else()
    set(BUILD_TYPE STATIC)
    list(APPEND CMAKE_CXX_FLAGS_DEBUG "-D_DEBUG")
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

WRITE_RCC_CONFIG("${RCC_INCLUDE_DEPENDENCIES}" "${RCC_LIBARY_DIRS_DEBUG}" "${RCC_LIBRARY_DIRS_RELEASE}" "${RCC_COMPILE_FLAGS}")

add_library(RuntimeCompiler ${BUILD_TYPE} ${RuntimeCompiler_SRCS})

add_library(RuntimeObjectSystem STATIC ${RuntimeObjectSystem_SRCS})
target_link_libraries(RuntimeObjectSystem RuntimeCompiler)

add_executable(test_tinterface "tests/test_tinterface.cpp")
target_link_libraries(test_tinterface RuntimeCompiler RuntimeObjectSystem)

if(NOT IObject_INCLUDE)
  set(IObject_INCLUDE Aurora/include)
endif(NOT IObject_INCLUDE)

target_include_directories(RuntimeCompiler 
  PUBLIC 
    $<BUILD_INTERFACE:${IObject_INCLUDE};
      ${CMAKE_CURRENT_SOURCE_DIR}/Aurora/RuntimeObjectSystem;
      ${CMAKE_CURRENT_SOURCE_DIR}/Aurora/RuntimeCompiler;
      ${CMAKE_CURRENT_SOURCE_DIR}/Aurora>
	$<INSTALL_INTERFACE:include>
)

target_include_directories(RuntimeObjectSystem 
  PUBLIC 
    $<BUILD_INTERFACE:${IObject_INCLUDE};
      ${CMAKE_CURRENT_SOURCE_DIR}/Aurora/RuntimeObjectSystem;
      ${CMAKE_CURRENT_SOURCE_DIR}/Aurora/RuntimeCompiler;
      ${CMAKE_CURRENT_SOURCE_DIR}/Aurora>
	  $<INSTALL_INTERFACE:include>
)

if(NOT WIN32)
    target_compile_options(RuntimeCompiler PRIVATE -fPIC -fvisibility=hidden)
    target_compile_options(RuntimeObjectSystem PRIVATE -fPIC -fvisibility=hidden)
endif(NOT WIN32)

export(TARGETS RuntimeCompiler RuntimeObjectSystem
      FILE "${PROJECT_BINARY_DIR}/RCCTargets.cmake")

export(PACKAGE RCC)

configure_file(rccConfig.cmake.in
  "${PROJECT_BINARY_DIR}/RCCConfig.cmake" @ONLY)

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

install(DIRECTORY cmake DESTINATION ./ FILES_MATCHING PATTERN "*.cmake")

install(FILES
  "${PROJECT_BINARY_DIR}/RCCConfig.cmake"
  DESTINATION "${CMAKE_INSTALL_PREFIX}" COMPONENT dev)

install(EXPORT RCCTargets DESTINATION "${CMAKE_INSTALL_PREFIX}" COMPONENT dev)

if(UNIX)
  target_link_libraries(RuntimeObjectSystem dl)
endif()
