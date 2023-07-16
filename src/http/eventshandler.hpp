#pragma once

#include <memory>

#include "handler.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Http
{
    class EventsHandler
    {
    public:
        explicit EventsHandler(ISession& session);
        ~EventsHandler();

        void operator()(uWS::HttpResponse<false>* res, uWS::HttpRequest* req);

    private:
        class State;
        std::shared_ptr<State> m_state;
    };
}
