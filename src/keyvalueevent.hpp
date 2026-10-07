#pragma once

#include <vector>

#include "event.hpp"

namespace porla
{
    struct KeyValueEvent : Event
    {
        using Event::Event;

        std::vector<std::string> keys;
    };
}
