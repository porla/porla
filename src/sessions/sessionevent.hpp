#pragma once

#include "../event.hpp"

namespace porla
{
    class Session;

    struct SessionEvent : Event
    {
        using Event::Event;

        int                    session_id = -1;
        std::weak_ptr<Session> session;
    };
}
