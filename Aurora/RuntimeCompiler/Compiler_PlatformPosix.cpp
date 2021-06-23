//
// Copyright (c) 2010-2011 Matthew Jack and Doug Binks
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

//
// Notes:
//   - We use a single intermediate directory for compiled .obj files, which means
//     we don't support compiling multiple files with the same name. Could fix this
//     with either mangling names to include paths,  or recreating folder structure
//
//

#include "Compiler.h"

#include <string>
#include <vector>
#include <iostream>
#include <set>

#include <fstream>
#include <sstream>

#include "assert.h"
#include <fcntl.h>
#include <sys/select.h>
#include <sys/wait.h>

#include "ICompilerLogger.h"

using namespace std;
const char	c_CompletionToken[] = "_COMPLETION_TOKEN_" ;

class PlatformCompilerImplData
{
public:
    PlatformCompilerImplData()
        : m_bCompileIsComplete( false )
        , m_pLogger( 0 )
        , m_ChildForCompilationPID( 0 )
    {
        m_PipeStdOut[0] = 0;
        m_PipeStdOut[1] = 1;
        m_PipeStdErr[0] = 0;
        m_PipeStdErr[1] = 1;
    }

    volatile bool		m_bCompileIsComplete;
    ICompilerLogger*	m_pLogger;
    pid_t               m_ChildForCompilationPID;
    int                 m_PipeStdOut[2];
    int                 m_PipeStdErr[2];
};

Compiler::Compiler()
    : m_pImplData( 0 )
{
}

Compiler::~Compiler()
{
    delete m_pImplData;
}

std::string Compiler::GetObjectFileExtension() const
{
    return ".o";
}

void Compiler::ReadFromPipe() const 
{
  if( m_pImplData->m_pLogger )
  {
    const size_t buffSize = 1024 * 80; //should allow for a few lines...
    char buffer[buffSize];
    fd_set set;
    struct timeval timeout;
    int rv = 0;
    FD_ZERO(&set); /* clear the set */
    FD_SET(m_pImplData->m_PipeStdOut[0], &set); /* add our file descriptor to the set */
    //FD_SET(m_pImplData->m_PipeStdErr[0], &set);
    
    
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;
    rv = select(m_pImplData->m_PipeStdOut[0] + 1,&set, nullptr, nullptr, &timeout);
    if(rv == -1)
    {
      m_pImplData->m_pLogger->LogError("An error occured reading from stdout");
    }
    ssize_t numread = 0;
    if(rv > 0)
    {
      while( ( numread = read( m_pImplData->m_PipeStdOut[0], buffer, buffSize-1 ) ) > 0 )
      {
        buffer[numread] = 0;
        m_pImplData->m_pLogger->LogWarning( buffer );
      }  
    }
    FD_ZERO(&set); /* clear the set */
    FD_SET(m_pImplData->m_PipeStdErr[0], &set); /* add our file descriptor to the set */
    rv = select(m_pImplData->m_PipeStdErr[0] + 1, &set, nullptr, nullptr, &timeout);
    if(rv == -1)
    {
      m_pImplData->m_pLogger->LogError("An error occured when reading from stderr");
    }
    if(rv > 0)
    {
      while( ( numread = read( m_pImplData->m_PipeStdErr[0], buffer, buffSize-1 ) )> 0 )
      {
        buffer[numread] = 0;
        m_pImplData->m_pLogger->LogError( buffer );
      }  
    }
  }
}

bool Compiler::GetIsComplete() const
{
    if( !m_pImplData->m_bCompileIsComplete && m_pImplData->m_ChildForCompilationPID )
    {

        // check for whether process is closed
        int procStatus;
        pid_t ret = waitpid( m_pImplData->m_ChildForCompilationPID, &procStatus, WNOHANG);
        if( ret && ( WIFEXITED(procStatus) || WIFSIGNALED(procStatus) ) )
        {
            m_pImplData->m_bCompileIsComplete = true;
            m_pImplData->m_ChildForCompilationPID = 0;

            // get output and log
            ReadFromPipe();

            // close the pipes as this process no longer needs them.
            close( m_pImplData->m_PipeStdOut[0] );
            m_pImplData->m_PipeStdOut[0] = 0;
            close( m_pImplData->m_PipeStdErr[0] );
            m_pImplData->m_PipeStdErr[0] = 0;
        }else {
            ReadFromPipe();
        }
    }
    return m_pImplData->m_bCompileIsComplete;
}

void Compiler::Initialise( ICompilerLogger * pLogger )
{
    m_pImplData = new PlatformCompilerImplData;
    m_pImplData->m_pLogger = pLogger;
}

void Compiler::RunCompile( const std::vector<FileSystemUtils::Path>&	filesToCompile_,
               const CompilerOptions&			compilerOptions_,
               std::vector<FileSystemUtils::Path>		linkLibraryList_,
                const FileSystemUtils::Path&		moduleName_ )

{
    const std::vector<FileSystemUtils::Path>& includeDirList = compilerOptions_.includeDirList;
    const std::vector<FileSystemUtils::Path>& libraryDirList = compilerOptions_.libraryDirList;
    const char* pCompileOptions =  compilerOptions_.compileOptions.c_str();
    const char* pLinkOptions = compilerOptions_.linkOptions.c_str();

    std::string compilerLocation = compilerOptions_.compilerLocation.m_string;
    std::string nvccCompiler;
    if (compilerLocation.size()==0){
#ifdef __clang__
        compilerLocation = "clang++ ";
#else // default to g++
        compilerLocation = "g++ ";
#endif //__clang__
    }else
    {
        std::string::size_type pos = compilerLocation.find(';');
        if(pos != std::string::npos) // found the delimiter
        {
            nvccCompiler = compilerLocation.substr(pos + 1);
            compilerLocation = compilerLocation.substr(0, pos);
        }
    }
    bool need_nvcc = false;
    for(const auto& file : filesToCompile_){
        if(file.Extension() == ".cu"){
            need_nvcc = true;
            break;
        }
    }
    if(!need_nvcc)
        nvccCompiler.clear();
    //NOTE: Currently doesn't check if a prior compile is ongoing or not, which could lead to memory leaks
    m_pImplData->m_bCompileIsComplete = false;

    //create pipes
    if ( pipe( m_pImplData->m_PipeStdOut ) != 0 )
    {
        
        if( m_pImplData->m_pLogger )
        {
            m_pImplData->m_pLogger->LogError( "Error in Compiler::RunCompile, cannot create pipe - perhaps insufficient memory?\n");
        }
        return;
    }
    fcntl(m_pImplData->m_PipeStdOut[0], F_SETFL, O_NONBLOCK);
    //create pipes
    if ( pipe( m_pImplData->m_PipeStdErr ) != 0 )
    {
        if( m_pImplData->m_pLogger )
        {
            m_pImplData->m_pLogger->LogError( "Error in Compiler::RunCompile, cannot create pipe - perhaps insufficient memory?\n");
        }
        return;
    }
    fcntl(m_pImplData->m_PipeStdErr[0], F_SETFL, O_NONBLOCK);
    const RCppOptimizationLevel optimizationLevel = GetActualOptimizationLevel( compilerOptions_.optimizationLevel );
    pid_t retPID;
    switch( retPID = fork() )
    {
        case -1: // error, no fork
            if( m_pImplData->m_pLogger )
            {
                m_pImplData->m_pLogger->LogError( "Error in Compiler::RunCompile, cannot fork() process - perhaps insufficient memory?\n");
            }
            return;
        case 0: // child process - carries on below.
            break;
        default: // current process - returns to allow application to run whilst compiling
            close( m_pImplData->m_PipeStdOut[1] );
            m_pImplData->m_PipeStdOut[1] = 0;
            close( m_pImplData->m_PipeStdErr[1] );
            m_pImplData->m_PipeStdErr[1] = 0;
            m_pImplData->m_ChildForCompilationPID = retPID;
           return;
    }

    //duplicate the pipe to stdout, so output goes to pipe
    dup2( m_pImplData->m_PipeStdErr[1], STDERR_FILENO );
    dup2( m_pImplData->m_PipeStdOut[1], STDOUT_FILENO );
    close( m_pImplData->m_PipeStdOut[0] );
    m_pImplData->m_PipeStdOut[0] = 0;
    close( m_pImplData->m_PipeStdErr[0] );
    m_pImplData->m_PipeStdErr[0] = 0;
    std::string compileString;
    if(nvccCompiler.size())
    {
#if __cplusplus > 201100L
        if(pCompileOptions){
            compileString = nvccCompiler + " -ccbin " + compilerLocation + " -g --compiler-options '-fPIC -fvisibility=hidden -shared ";
            std::stringstream ss;
            ss << pCompileOptions;
            std::string op;
            while(std::getline(ss, op, ' ')){
                if(op.find("-Wl") == std::string::npos){
                    compileString += op + " ";
                }
            }
            compileString += "' ";
        }else{
            compileString = nvccCompiler + " -ccbin " + compilerLocation + " -g --compiler-options '-fPIC -fvisibility=hidden -shared' ";
        }

#else
        compileString = nvccCompiler + " -ccbin " + compilerLocation + " -g --compiler-options '-fPIC -fvisibility=hidden -shared' ";
#endif
    }else
    {
#if __cplusplus > 201100L
    compileString = compilerLocation + " -g -fPIC -fvisibility=hidden -shared ";
#else
    compileString = compilerLocation + " " + "-g -fPIC -fvisibility=hidden -shared ";
#endif
    }

#ifndef __LP64__
    compileString += "-m32 ";
#endif

    
    switch( optimizationLevel )
    {
    case RCCPPOPTIMIZATIONLEVEL_DEFAULT:
        assert(false);
    case RCCPPOPTIMIZATIONLEVEL_DEBUG:
        compileString += "-O0 ";
        break;
    case RCCPPOPTIMIZATIONLEVEL_PERF:
        compileString += "-O2 -DNDEBUG ";
        break;
    case RCCPPOPTIMIZATIONLEVEL_NOT_SET:;

    case RCCPPOPTIMIZATIONLEVEL_SIZE:
        compileString += "-O3 ";
        break;
    }

    // Check for intermediate directory, create it if required
    // There are a lot more checks and robustness that could be added here
    if( !compilerOptions_.intermediatePath.Exists() )
    {
        bool success = compilerOptions_.intermediatePath.CreateDir();
        if( success && m_pImplData->m_pLogger ) { m_pImplData->m_pLogger->LogInfo("Created intermediate folder \"%s\"\n", compilerOptions_.intermediatePath.c_str()); }
        else if( m_pImplData->m_pLogger ) { m_pImplData->m_pLogger->LogError("Error creating intermediate folder \"%s\"\n", compilerOptions_.intermediatePath.c_str()); }
    }

    FileSystemUtils::Path	output = moduleName_;
    bool bCopyOutput = false;
    if( compilerOptions_.intermediatePath.Exists() )
    {
        // add save object files
        compileString = "cd \"" + compilerOptions_.intermediatePath.m_string + "\"\n" + compileString + " ";
        output = compilerOptions_.intermediatePath / "a.out";
        bCopyOutput = true;
    }


    // include directories
    std::set<std::string> uniqueIncludes;

    for( size_t i = 0; i < includeDirList.size(); ++i )
    {
        uniqueIncludes.insert(includeDirList[i].m_string);
        //compileString += "-I\"" + includeDirList[i].m_string + "\" ";
    }
    for(std::set<std::string>::const_iterator it = uniqueIncludes.cbegin(); it != uniqueIncludes.cend(); ++it)
    {
        compileString += "-I\"" + *it + "\" ";
    }

    // library and framework directories
    for( size_t i = 0; i < libraryDirList.size(); ++i )
    {
        compileString += "-L\"" + libraryDirList[i].m_string + "\" ";
        if(nvccCompiler.empty())
            compileString += "-F\"" + libraryDirList[i].m_string + "\" ";
    }

    if( !bCopyOutput )
    {
        // output file
        compileString += "-o " + output.m_string + " ";
    }

    if( pCompileOptions && nvccCompiler.empty() )
    {
        compileString += pCompileOptions;
        compileString += " ";
    }
    if( pLinkOptions && strlen(pLinkOptions) )
    {
        if(nvccCompiler.empty())
            compileString += "-Wl,";
        compileString += pLinkOptions;
        compileString += " ";
    }

    // files to compile
    for( size_t i = 0; i < filesToCompile_.size(); ++i )
    {
        compileString += "\"" + filesToCompile_[i].m_string + "\" ";
    }
    std::set<std::string> uniqueLibraries;
    for( size_t i = 0; i < linkLibraryList_.size(); ++i )
    {
        uniqueLibraries.insert(linkLibraryList_[i].m_string);
    }
    // libraries to link
    for( std::set<std::string>::const_iterator it = uniqueLibraries.begin(); it != uniqueLibraries.end(); ++it)
    {
        compileString += " " + *it + " ";
    }

    for(size_t i = 0; i < compilerOptions_.linkLibraries.size(); ++i)
    {
        compileString += " " + compilerOptions_.linkLibraries[i];
    }
    if(RCCPPOPTIMIZATIONLEVEL_DEBUG == optimizationLevel)
    {
        for(size_t i = 0; i < compilerOptions_.debugLinkLibraries.size(); ++i)
        {
            compileString += " " + compilerOptions_.debugLinkLibraries[i];
        }
    }else
    {
        for(size_t i = 0; i < compilerOptions_.releaseLinkLibraries.size(); ++i)
        {
            compileString += " " + compilerOptions_.releaseLinkLibraries[i];
        }
    }

    if( bCopyOutput )
    {
        compileString += "\n mv \"" + output.m_string + "\" \"" + moduleName_.m_string + "\"\n";
    }


    std::cout << compileString << std::endl << std::endl;

    execl("/bin/sh", "sh", "-c", compileString.c_str(), (const char*)NULL);
}

bool Compiler::AbortCompile()
{
    kill(m_pImplData->m_ChildForCompilationPID, SIGTERM);
    return true;
}
