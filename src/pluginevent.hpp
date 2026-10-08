#pragma once

#include "event.hpp"

namespace porla
{
    struct PluginEvent : Event
    {
        using Event::Event;

        int plugin_id = -1;
    };
}
