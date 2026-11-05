#pragma once

#include <optional>
#include <string_view>

#define PORLA_ERRORS(X)                                       \
    X(parse_error,       -32700, "Parse error")               \
    X(invalid_request,   -32600, "Invalid Request")           \
    X(method_not_found,  -32601, "Method not found")          \
    X(invalid_params,    -32602, "Invalid params")            \
    X(internal_error,    -32603, "Internal error")            \
    X(permission_denied,   1001, "Invocation not allowed")    \
    X(libtorrent_error,   -5000, "libtorrent error")

namespace porla::Errors
{
    enum class Code
    {
#define PORLA_ERRORS_ENUM(name, rpc_code, message) name,
        PORLA_ERRORS(PORLA_ERRORS_ENUM)
#undef PORLA_ERRORS_ENUM
    };

    struct Entry
    {
        std::string_view name;
        int              rpc_code;
        std::string_view message;
    };

    constexpr Entry Get(Code code)
    {
        switch (code)
        {
#define PORLA_ERRORS_CASE(name, rpc_code, message) case Code::name: return { #name, rpc_code, message };
            PORLA_ERRORS(PORLA_ERRORS_CASE)
#undef PORLA_ERRORS_CASE
        }

        return { "internal_error", -32603, "Internal error" };
    }

    constexpr std::optional<Code> Find(std::string_view name)
    {
#define PORLA_ERRORS_FIND(n, rpc_code, message) if (name == #n) { return Code::n; }
        PORLA_ERRORS(PORLA_ERRORS_FIND)
#undef PORLA_ERRORS_FIND

        return std::nullopt;
    }
}
