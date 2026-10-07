#pragma once

#include "event.hpp"

namespace porla
{
    struct PresetEvent : Event
    {
        using Event::Event;

        int preset_id = -1;
    };
}
