include(CheckLibraryExists)
include(FetchContent)

check_library_exists(m cos "" HAVE_LIB_M)

FetchContent_Declare(
    duktape
    GIT_REPOSITORY https://github.com/svaarala/duktape-releases.git
    GIT_TAG "v2.7.0")

FetchContent_MakeAvailable(duktape)

if (HAVE_LIB_M)
    set(DUKTAPE_DEPENDENCIES ${EXTRA_LIBS} m)
endif()

# Sources
set(DUKTAPE_SRC_DIR    ${duktape_SOURCE_DIR}/src)
set(DUKTAPE_EXTRAS_DIR ${duktape_SOURCE_DIR}/extras)
set(DUKTAPE_SRC        ${DUKTAPE_SRC_DIR}/duktape.c)

add_library(duktape STATIC ${DUKTAPE_SRC})

target_include_directories(duktape PUBLIC ${DUKTAPE_SRC_DIR})
target_include_directories(duktape PUBLIC ${DUKTAPE_EXTRAS_DIR})
target_link_libraries(duktape PUBLIC ${DUKTAPE_DEPENDENCIES})