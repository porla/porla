#pragma once

#include <functional>

#include <uWebSockets/App.h>

namespace porla::Http
{
    template <bool SSL> using Handler = std::function<void((uWS::HttpResponse<SSL> * , uWS::HttpRequest *))>;
}
