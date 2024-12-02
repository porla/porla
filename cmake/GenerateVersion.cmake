execute_process(
    COMMAND git branch --show-current
    OUTPUT_VARIABLE GIT_BRANCH
    ERROR_QUIET)

execute_process(
    COMMAND git rev-parse HEAD
    OUTPUT_VARIABLE GIT_COMMITISH
    ERROR_QUIET)

if ("${GIT_BRANCH}" STREQUAL "")
    set(GIT_BRANCH "-")
endif()

if ("${GIT_COMMITISH}" STREQUAL "")
    set(GIT_COMMITISH "-")
endif()

set(VERSION "$ENV{GITVERSION_SEMVER}")
set(VERSION_MAJOR "$ENV{GITVERSION_MAJOR}")
set(VERSION_MINOR "$ENV{GITVERSION_MINOR}")
set(VERSION_PATCH "$ENV{GITVERSION_PATCH}")

if ("${VERSION_MAJOR}" STREQUAL "")
    set(VERSION_MAJOR "0")
endif()

if ("${VERSION_MINOR}" STREQUAL "")
    set(VERSION_MINOR "0")
endif()

if ("${VERSION_PATCH}" STREQUAL "")
    set(VERSION_PATCH "0")
endif()

string(STRIP "${GIT_BRANCH}" GIT_BRANCH)
string(STRIP "${GIT_COMMITISH}" GIT_COMMITISH)

set(VERSION_SRC "
const char* GIT_BRANCH=\"${GIT_BRANCH}\";
const char* GIT_COMMITISH=\"${GIT_COMMITISH}\";
const char* VERSION=\"${VERSION}\";
const char* VERSION_MAJOR=\"${VERSION_MAJOR}\";
const char* VERSION_MINOR=\"${VERSION_MINOR}\";
const char* VERSION_PATCH=\"${VERSION_PATCH}\";
")

message(VERSION_SRC="${VERSION_SRC}")

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp)
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp VERSION_SRC_)
else()
    set(VERSION_SRC_ "")
endif()

if (NOT "${VERSION_SRC}" STREQUAL "${VERSION_SRC_}")
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp "${VERSION_SRC}")
endif()
