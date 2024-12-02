#pragma once

#include <functional>

#include <uWebSockets/App.h>

namespace porla::Http
{
    typedef std::function<void((uWS::HttpResponse<false> * , uWS::HttpRequest *))> Handler;
}
