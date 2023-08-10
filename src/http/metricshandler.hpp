#pragma once

#include <memory>

#include "handler.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Http
{
    class MetricsHandler
    {
    public:
        explicit MetricsHandler(Sessions& sessions);
        ~MetricsHandler();

        void operator()(uWS::HttpResponse<false>* res, [[maybe_unused]] uWS::HttpRequest* req);

    private:
        class State;
        std::shared_ptr<State> m_state;
    };
}
