#pragma once

#include <memory>

#include "handler.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Http
{
    class EventsHandler
    {
    public:
        explicit EventsHandler(Sessions& sessions);
        ~EventsHandler();

        void operator()(uWS::HttpResponse<true>* res, uWS::HttpRequest* req);

    private:
        class State;
        std::shared_ptr<State> m_state;
    };
}
