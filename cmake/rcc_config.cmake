
# helper function to recursively get dependent library directories and includes

macro(_link_lib_helper LIB_VAR INC_VAR tgt )

endmacro(_link_lib_helper)

macro(_target_helper LIB_VAR INC_VAR tgt tab)
    if(TARGET ${tgt})
        message(${tab}${tgt})
        get_target_property(int_link_lib ${tgt} INTERFACE_LINK_LIBRARIES)
        foreach(lib ${int_link_lib})
            if(TARGET ${lib})
                _target_helper(${LIB_VAR} ${INC_VAR} ${lib} "${tab}  ")
            else(TARGET ${lib})

            endif(TARGET ${lib})
        endforeach(lib ${int_link_lib})
        get_target_property(out_dir ${tgt} LIBRARY_OUTPUT_DIRECTORY)
        if(out_dir)
            list(APPEND ${LIB_VAR} ${out_dir})
        endif(out_dir)
        get_target_property(inc_dir ${tgt} INTERFACE_INCLUDE_DIRECTORIES)
        if(inc_dir)
            list(APPEND ${INC_VAR} ${inc_dir})
        endif()
        get_target_property(inc_dir ${tgt} INCLUDE_DIRECTORIES)
        if(inc_dir)
            list(APPEND ${INC_VAR} ${inc_dir})
        endif()
    else(TARGET ${tgt})
        message("${tgt} is not a target")
    endif(TARGET ${tgt})
endmacro(_target_helper)

macro(RCC_TARGET_CONFIG target)



endmacro(RCC_TARGET_CONFIG)




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
