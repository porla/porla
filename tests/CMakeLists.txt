FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/releases/download/v1.18.0/googletest-1.18.0.tar.gz
    EXCLUDE_FROM_ALL
)

set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(BUILD_GMOCK   OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_executable(
    porla_tests
    events.cpp
    logging.cpp
    query/pql.cpp
    utils/string.cpp
)

target_link_libraries(porla_tests PRIVATE porla_core GTest::gtest_main)

gtest_discover_tests(
    porla_tests
    DISCOVERY_MODE PRE_TEST
    PROPERTIES LABELS unit ENVIRONMENT "TZ=Europe/Stockholm")
