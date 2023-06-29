#pragma once

#include <functional>

#include "../../vendor/uWebSockets/src/App.h"

namespace porla::Http
{
    typedef std::function<void((uWS::HttpResponse<false> * , uWS::HttpRequest *))> Handler;
}
