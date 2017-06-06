# helper function to recursively get dependent library directories and includes

macro(__target_helper LIB_DIR_VAR INC_VAR LIB_FILES tgt tab TGTS)
    list(FIND ${TGTS} ${tgt} index_)
    if(${index_} EQUAL -1)
        if(RCC_VERBOSE_CONFIG)
            message(STATUS "${tab}${tgt}")
        endif(RCC_VERBOSE_CONFIG)
        if(TARGET ${tgt})
            list(APPEND ${TGTS} ${tgt})
            get_target_property(imported_ ${tgt} IMPORTED)
            if(${imported_})
                get_target_property(tgt_location ${tgt} LOCATION)
                get_filename_component(name_ ${tgt_location} NAME_WE)
                get_filename_component(dir_ ${tgt_location} DIRECTORY)
                LIST(APPEND ${LIB_DIR_VAR} ${dir_})
                LIST(APPEND ${LIB_FILES} ${name_})
            endif(${imported_})
            get_target_property(int_link_lib ${tgt} INTERFACE_LINK_LIBRARIES)
            foreach(lib ${int_link_lib})
                if(TARGET ${lib})
                    __target_helper(${LIB_DIR_VAR} ${INC_VAR} ${LIB_FILES} ${lib} "${tab}  " ${TGTS})
                else(TARGET ${lib})
                    if(EXISTS ${lib})
                        get_filename_component(dir_ ${lib} DIRECTORY)
                        list(APPEND ${LIB_DIR_VAR} ${dir_})
                        get_filename_component(name_ ${lib} NAME_WE)
                        list(APPEND ${LIB_FILES} ${name_})
                    endif(EXISTS ${lib})
                endif(TARGET ${lib})
            endforeach(lib ${int_link_lib})

            get_target_property(int_link_lib ${tgt} LINK_LIBRARIES)
            foreach(lib ${int_link_lib})
                if(TARGET ${lib})
                    __target_helper(${LIB_DIR_VAR} ${INC_VAR} ${LIB_FILES} ${lib} "${tab}  " ${TGTS})
                else(TARGET ${lib})
                    if(EXISTS ${lib})
                        get_filename_component(dir_ ${lib} DIRECTORY)
                        list(APPEND ${LIB_DIR_VAR} ${dir_})
                        get_filename_component(name_ ${lib} NAME_WE)
                        list(APPEND ${LIB_FILES} ${name_})
                    endif(EXISTS ${lib})
                endif(TARGET ${lib})
            endforeach(lib ${int_link_lib})

            get_target_property(out_dir ${tgt} LIBRARY_OUTPUT_DIRECTORY)
            if(out_dir)
                list(APPEND ${LIB_DIR_VAR} ${out_dir})
            endif(out_dir)
            get_target_property(lib_path ${tgt} LIBRARY_OUTPUT_DIRECTORY)
            if(lib_path)
                list(APPEND ${LIB_DIR_VAR} ${out_dir})
            endif(lib_path)
            get_target_property(inc_dir ${tgt} INTERFACE_INCLUDE_DIRECTORIES)
            if(inc_dir)
                list(APPEND ${INC_VAR} ${inc_dir})
            endif()
            get_target_property(inc_dir ${tgt} INCLUDE_DIRECTORIES)
            if(inc_dir)
                list(APPEND ${INC_VAR} ${inc_dir})
            endif()
            get_target_property(imp ${tgt} IMPORTED)
            if(imp)
                get_target_property(imp_lib ${tgt} LOCATION)
                get_filename_component(dir ${imp_lib} DIRECTORY)
                list(APPEND ${LIB_DIR_VAR} ${dir})
            endif(imp)
        else(TARGET ${tgt})
            # not a target but a file
            if(EXISTS ${tgt})
                get_filename_component(dir_ ${tgt} DIRECTORY)
                list(APPEND ${LIB_DIR_VAR} ${dir_})
                get_filename_component(name_ ${tgt} NAME_WE)
                list(APPEND ${LIB_FILES} ${name_})
            endif(EXISTS ${tgt})
        endif(TARGET ${tgt})
    endif(${index_} EQUAL -1)
endmacro(__target_helper)

macro(_target_helper LIB_DIR_VAR INC_VAR LIB_FILES tgt tab)
    set(TGT_LIST "")
    __target_helper(${LIB_DIR_VAR} ${INC_VAR} ${LIB_FILES} ${tgt} ${tab} TGT_LIST)
endmacro(_target_helper)

macro(RCC_TARGET_CONFIG target LIB_FILES_VAR)
    set(inc_dirs "")
    set(lib_dirs "")
    set(${LIB_FILES_VAR} "")
    set(flags "")
    set(defs "")
    _target_helper(lib_dirs inc_dirs ${LIB_FILES_VAR} ${target} "  ")

    get_target_property(dest_dir ${target} CMAKE_ARCHIVE_OUTPUT_DIRECTORY)

    get_target_property(flags_ ${target} COMPILE_OPTIONS)
    if(flags_)
        set(flags ${flags_})
    endif()
    get_directory_property(flags_ ${CMAKE_CURRENT_SOURCE_DIR} COMPILE_OPTIONS)
    if(flags_)
        set(flags ${flags_})
    endif()

    get_target_property(defs_ ${target} COMPILE_DEFINITIONS)
    if(defs_)
        set(defs "${defs};${defs_}")
    endif()
    get_directory_property(defs_ DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMPILE_DEFINITIONS)
    if(defs_)
        set(defs "${defs};${defs_}")
    endif()
    set(NVCC_COMPILER "")
    IF(CUDA_FOUND)
        set(NVCC_COMPILER "${CUDA_NVCC_EXECUTABLE}")
    ENDIF()
    set(COMPILER_PATH ${CMAKE_CXX_COMPILER};${NVCC_COMPILER})
    list(REMOVE_DUPLICATES inc_dirs)
    list(REMOVE_DUPLICATES flags)
    list(REMOVE_DUPLICATES defs)
    list(REMOVE_DUPLICATES lib_dirs)

    set(inc "")
    foreach(dir "${inc_dirs}")
        set(inc "${dir}\n")
    endforeach(dir)
    set(lib "")
    foreach(dir "${lib_dirs}")
        set(lib "${dir}\n")
    endforeach(dir)
    if(WIN32)
        list(APPEND flags "/FS")
    endif(WIN32)
    string(REGEX REPLACE ";" "\n" inc "${inc}")
    string(REGEX REPLACE ";" "\n" lib "${lib}")
    string(REGEX REPLACE ";" "\n" flags "${flags}")
    string(REGEX REPLACE ";" "\n" defs "${defs}")

    message(STATUS "Writing ${dest_dir}/${target}_config.txt")
    FILE(WRITE "${dest_dir}/${target}_config.txt"
        "project_id:\n0\n\n"
        "include_dirs:\n${inc}\n"
        "lib_dirs_debug:\n${lib}\n"
        "lib_dirs_release:\n${lib}\n"
        "compile_options:\n${flags}\n\n"
        "compile_definitions:\n${defs}\n\n"
        "compiler_location:\n${COMPILER_PATH}"
    )
endmacro(RCC_TARGET_CONFIG)

macro(WRITE_RCC_CONFIG RCC_INCLUDE_DEPENDENCIES RCC_LIBRARY_DIRS_DEBUG RCC_LIBRARY_DIRS_RELEASE RCC_COMPILE_FLAGS)

    string(REGEX REPLACE ";" "\n" includes_ "${RCC_INCLUDE_DEPENDENCIES}")
    #string(REGEX REPLACE "$<BUILD_INTERFACE:" "" includes_ "${includes_}")
    #string(REGEX REPLACE "" "" includes_ "${includes_}")
    string(REGEX REPLACE ";" "\n" lib_dirs_deb_ "${RCC_LIBRARY_DIRS_DEBUG}")
    string(REGEX REPLACE ";" "\n" lib_dirs_rel_ "${RCC_LIBRARY_DIRS_RELEASE}")
    if(RCC_COMPILE_FLAGS)
        string(REGEX REPLACE ";" "\n" flags_ "${RCC_COMPILE_FLAGS}")
    else()
        set(flags_ "")
    endif()
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
    ELSE(WIN32)
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
    ENDIF(WIN32)
endmacro()
