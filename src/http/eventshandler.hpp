#pragma once

#include <memory>

#include "handler.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Http
{
    
    template <bool SSL> class EventsHandler
    {
    public:
        explicit EventsHandler(Sessions& sessions);
        ~EventsHandler();

        void operator()(uWS::HttpResponse<SSL>* res, uWS::HttpRequest* req);

    private:
        template <bool SSLS> class State;
        std::shared_ptr<State<SSL>> m_state;
    };
}
