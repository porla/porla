#pragma once

#include <optional>

namespace porla::Utils
{
    inline std::optional<int> CheckRateLimit(int value)
    {
        if (value < -1)
        {
            return std::nullopt;
        }

        return value;
    }

    inline std::optional<int> CheckPeerLimit(int value)
    {
        if (value == 0)
        {
            return -1;
        }

        if (value == -1 || value >= 2)
        {
            return value;
        }

        return std::nullopt;
    }
}
