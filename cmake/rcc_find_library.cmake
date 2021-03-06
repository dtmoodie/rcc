macro(rcc_find_library var_name)
    find_library(${var_name} ${ARGN})
    if(RCC_VERBOSE_CONFIG)
        if(${var_name})
            message(STATUS "${var_name} found at ${${var_name}}")
        else(${var_name})
            message(STATUS "${var_name} not found")
        endif(${var_name})
    endif(RCC_VERBOSE_CONFIG)
endmacro(rcc_find_library var_name)