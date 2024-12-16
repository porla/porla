#pragma once

#include <functional>

#include <uWebSockets/App.h>

namespace porla::Http
{
    typedef std::function<void((uWS::HttpResponse<true> * , uWS::HttpRequest *))> Handler;
}
