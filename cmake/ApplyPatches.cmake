set(APPLIED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.porla-patches")

file(GLOB PATCHES "${PATCH_DIR}/*.patch")
file(GLOB APPLIED "${APPLIED_DIR}/*.patch")
list(SORT PATCHES)
list(SORT APPLIED)

set(WANTED "")
set(HAVE "")

foreach(PATCH IN LISTS PATCHES)
    get_filename_component(NAME "${PATCH}" NAME)
    file(SHA256 "${PATCH}" HASH)
    list(APPEND WANTED "${NAME}:${HASH}")
endforeach()

foreach(PATCH IN LISTS APPLIED)
    get_filename_component(NAME "${PATCH}" NAME)
    file(SHA256 "${PATCH}" HASH)
    list(APPEND HAVE "${NAME}:${HASH}")
endforeach()

if ("${WANTED}" STREQUAL "${HAVE}")
    message(STATUS "Patches up to date")
    return()
endif()

# this runs again on an already patched tree whenever a patch is added, changed or removed, so take off what
# was applied before (newest first) and apply the current set
list(REVERSE APPLIED)

foreach(PATCH IN LISTS APPLIED)
    get_filename_component(NAME "${PATCH}" NAME)

    execute_process(
        COMMAND git apply --reverse "${PATCH}"
        RESULT_VARIABLE RESULT
        ERROR_VARIABLE ERROR)

    if (NOT RESULT EQUAL 0)
        message(FATAL_ERROR "Patch ${NAME} could not be reverted, delete ${CMAKE_CURRENT_SOURCE_DIR} and configure again:\n${ERROR}")
    endif()

    file(REMOVE "${PATCH}")
    message(STATUS "Patch ${NAME}: reverted")
endforeach()

foreach(PATCH IN LISTS PATCHES)
    get_filename_component(NAME "${PATCH}" NAME)

    execute_process(
        COMMAND git apply "${PATCH}"
        RESULT_VARIABLE RESULT
        ERROR_VARIABLE ERROR)

    if (NOT RESULT EQUAL 0)
        message(FATAL_ERROR "Patch ${NAME} does not apply:\n${ERROR}")
    endif()

    file(COPY "${PATCH}" DESTINATION "${APPLIED_DIR}")
    message(STATUS "Patch ${NAME}: applied")
endforeach()
