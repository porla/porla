# FindANTLR4-Runtime.cmake
# Place this file in your CMAKE_MODULE_PATH directory (e.g., cmake/modules/)

# Find ANTLR4 library and include paths
find_path(ANTLR4_INCLUDE_DIR
    NAMES antlr4-runtime.h
    PATH_SUFFIXES antlr4-runtime
    PATHS
        /usr/include
        /usr/local/include
        ${ANTLR4_ROOT}/include
)

find_library(ANTLR4_LIBRARY
    NAMES antlr4-runtime libantlr4-runtime
    PATHS
        /usr/lib
        /usr/local/lib
        ${ANTLR4_ROOT}/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ANTLR4
    REQUIRED_VARS
        ANTLR4_INCLUDE_DIR
        ANTLR4_LIBRARY
)

if(ANTLR4_FOUND)
    set(ANTLR4_INCLUDE_DIRS ${ANTLR4_INCLUDE_DIR})
    set(ANTLR4_LIBRARIES ${ANTLR4_LIBRARY})
    
    if(NOT TARGET antlr4_shared)
        add_library(antlr4_shared SHARED IMPORTED)
        set_target_properties(antlr4_shared PROPERTIES
            IMPORTED_LOCATION "${ANTLR4_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${ANTLR4_INCLUDE_DIR}"
        )
    endif()
endif()

mark_as_advanced(
    ANTLR4_INCLUDE_DIR
    ANTLR4_LIBRARY
)