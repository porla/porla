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

        void operator()(uWS::HttpResponse<false>* res, uWS::HttpRequest* req, const jwt::decoded_jwt<jwt::traits::nlohmann_json>& token);

    private:
        class State;
        std::shared_ptr<State> m_state;
    };
}
