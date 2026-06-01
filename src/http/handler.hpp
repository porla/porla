#pragma once

#include <functional>

#include <jwt-cpp/traits/nlohmann-json/defaults.h>
#include <jwt-cpp/jwt.h>
#include <uWebSockets/App.h>

namespace porla::Http
{
    typedef std::function<void((uWS::HttpResponse<false> * , uWS::HttpRequest *))> Handler;
    typedef std::function<void((uWS::HttpResponse<false> * , uWS::HttpRequest *, const jwt::decoded_jwt<jwt::traits::nlohmann_json>& token))> SecureHandler;
}
