#pragma once

#include <optional>
#include <string_view>

#define PORLA_ERRORS(X)                                             \
    X(parse_error,            -32700, "Parse error")                \
    X(invalid_request,        -32600, "Invalid Request")            \
    X(method_not_found,       -32601, "Method not found")           \
    X(invalid_params,         -32602, "Invalid params")             \
    X(internal_error,         -32603, "Internal error")             \
    X(permission_denied,        1001, "Invocation not allowed")     \
    X(session_not_found,          -1, "Session not found")          \
    X(session_not_loaded,         -2, "Session not loaded")         \
    X(torrent_not_found,          -3, "Torrent not found")          \
    X(torrent_already_added,     -10, "Torrent already in session") \
    X(torrent_no_metadata,       -11, "Torrent has no metadata")    \
    X(torrent_add_failed,        -12, "Failed to add torrent")      \
    X(preset_not_found,          -20, "Preset not found")           \
    X(already_registered,        -48, "Already registered")         \
    X(decode_error,              -62, "Decode error")               \
    X(decrypt_failed,            -63, "Decryption failed")          \
    X(invalid_state,             -64, "Invalid state")              \
    X(closed,                    -65, "Connection closed")          \
    X(libtorrent_error,        -5000, "libtorrent error")           \
    X(io_error,                -5001, "I/O error")                  \
    X(http_error,              -5002, "HTTP error")

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
