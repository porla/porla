execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    OUTPUT_VARIABLE GIT_BRANCH
    ERROR_QUIET)

execute_process(
    COMMAND git log --pretty=format:'%h' -n 1
    OUTPUT_VARIABLE GIT_COMMITISH
    ERROR_QUIET)

if ("${GIT_BRANCH}" STREQUAL "")
    set(GIT_BRANCH "-")
endif()

if ("${GIT_COMMITISH}" STREQUAL "")
    set(GIT_COMMITISH "-")
else()
    string(STRIP "${GIT_COMMITISH}" GIT_COMMITISH)
    string(SUBSTRING "${GIT_COMMITISH}" 1 7 GIT_COMMITISH)
endif()

set(VERSION "$ENV{GITVERSION_SEMVER}")

string(STRIP "${GIT_BRANCH}" GIT_BRANCH)
string(STRIP "${GIT_COMMITISH}" GIT_COMMITISH)

set(VERSION_SRC "const char* GIT_BRANCH=\"${GIT_BRANCH}\";
const char* GIT_COMMITISH=\"${GIT_COMMITISH}\";
const char* VERSION=\"${VERSION}\";")

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp)
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp VERSION_SRC_)
else()
    set(VERSION_SRC_ "")
endif()

if (NOT "${VERSION_SRC}" STREQUAL "${VERSION_SRC_}")
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp "${VERSION_SRC}")
endif()
