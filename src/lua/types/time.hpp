#pragma once

#include <cstdint>
#include <ctime>
#include <optional>

namespace porla::Lua::Types
{
    inline std::optional<std::int64_t> NonZeroTime(std::time_t t)
    {
        if (t == 0) return std::nullopt;
        return static_cast<std::int64_t>(t);
    }
}
