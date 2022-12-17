
# Custom command that will only build stdlib when it changes.
add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/vendor/jsonnet/core/std.jsonnet.h
    COMMAND ${PROJECT_BINARY_DIR}/to_c_array
        ${PROJECT_SOURCE_DIR}/vendor/jsonnet/stdlib/std.jsonnet
        ${PROJECT_SOURCE_DIR}/vendor/jsonnet/core/std.jsonnet.h
    DEPENDS to_c_array stdlib/std.jsonnet)

# Standard library build target that libjsonnet can depend on.
add_custom_target(stdlib ALL DEPENDS ${PROJECT_SOURCE_DIR}/vendor/jsonnet/core/std.jsonnet.h)

add_library(
    jsonnet
    STATIC
    core/ast.h
    core/desugarer.cpp
    core/desugarer.h
    core/formatter.cpp
    core/formatter.h
    core/json.h
    core/lexer.cpp
    core/lexer.h
    core/libjsonnet.cpp
    core/parser.cpp
    core/parser.h
    core/pass.cpp
    core/pass.h
    core/state.h
    core/static_analysis.cpp
    core/static_analysis.h
    core/static_error.h
    core/string_utils.cpp
    core/string_utils.h
    core/unicode.h
    core/vm.cpp
    core/vm.h

    # cpp
    cpp/libjsonnet++.cpp

    # third party
    third_party/md5/md5.cpp
    third_party/md5/md5.h
)

target_include_directories(
    jsonnet
    PRIVATE
    third_party/md5
)

target_include_directories(
    jsonnet
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    jsonnet
    nlohmann_json::nlohmann_json
    ryml::ryml
)

add_dependencies(
    jsonnet
    stdlib
)
