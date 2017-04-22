macro(WRITE_RCC_CONFIG RCC_INCLUDE_DEPENDENCIES RCC_LIBRARY_DIRS_DEBUG RCC_LIBRARY_DIRS_RELEASE RCC_COMPILE_FLAGS)

string(REGEX REPLACE ";" "\n" includes_ "${RCC_INCLUDE_DEPENDENCIES}")
string(REGEX REPLACE ";" "\n" lib_dirs_deb_ "${RCC_LIBRARY_DIRS_DEBUG}")
string(REGEX REPLACE ";" "\n" lib_dirs_rel_ "${RCC_LIBRARY_DIRS_RELEASE}")
string(REGEX REPLACE ";" "\n" flags_ "${RCC_COMPILE_FLAGS}")
#string(REGEX REPLACE ";" "\n" lib_dirs_reldeb_ "${RCC_LIBRARY_DIRS_RELWITHDEBINFO}")
set(NVCC_COMPILER "")
IF(CUDA_FOUND)
    set(NVCC_COMPILER "${CUDA_NVCC_EXECUTABLE}")
ENDIF()
set(COMPILER_PATH ${CMAKE_CXX_COMPILER};${NVCC_COMPILER})
IF(WIN32)
    FILE(WRITE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/RCC_config.txt" 
        "project_id:\n0\n"
        "include_dirs:\n${includes_}\n"
        "lib_dirs_debug:\n${lib_dirs_deb_}\n"
        "compile_options:\n/FS ${flags_}\n"
        "compiler_locations:\n${COMPILER_PATH}")

    FILE(WRITE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/RCC_config.txt" 
        "project_id:\n0"
        "\ninclude_dirs:\n${includes_}"
        "\nlib_dirs_release:\n${lib_dirs_rel_}"
        "\ncompile_options:\n/FS ${flags_}"
        "\ncompiler_location:\n${COMPILER_PATH}")
	
    FILE(WRITE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/RelWithDebInfo/RCC_config.txt" 
        "project_id:\n0"
        "\ninclude_dirs:\n${includes_}"
        "lib_dirs_release:\n${lib_dirs_rel_}"
        "compile_options:\n/FS ${flags_}"
        "compiler_location:\n${COMPILER_PATH}")
	
    FILE(WRITE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/RCC_config.txt" 
        "project_id:\n0"
        "\ninclude_dirs:\n${includes_}"
        "\nlib_dirs_debug:\n${lib_dirs_deb_}"
        "\nlib_dirs_release:\n${lib_dirs_rel_}"
        "\ncompile_options:\n\n/FS ${flags_}"
        "\ncompiler_location:\n${COMPILER_PATH}")
ELSE()
    FILE(WRITE "${CMAKE_BINARY_DIR}/RCC_config.txt"
        "project_id:\n0\n"
        "\ninclude_dirs:\n${includes_}\n"
        "\nlib_dirs_debug:\n${lib_dirs_deb_}\n${CMAKE_BINARY_DIR}\n"
        "\nlib_dirs_release:\n${lib_dirs_rel_}\n${CMAKE_BINARY_DIR}"
        "\ncompile_options:\n${flags_}\n"
        "\ncompiler_location:\n${COMPILER_PATH}")

    FILE(WRITE "${CMAKE_CURRENT_BINARY_DIR}/RCC_config.txt"
        "project_id:\n0\n"
        "\ninclude_dirs:\n${includes_}\n"
        "\nlib_dirs_debug:\n${lib_dirs_deb_}\n${CMAKE_BINARY_DIR}\n"
        "\nlib_dirs_release:\n${lib_dirs_rel_}\n${CMAKE_BINARY_DIR}"
        "\ncompile_options:\n${flags_}\n"
        "\ncompiler_location:\n${COMPILER_PATH}")
ENDIF()
endmacro()