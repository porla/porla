#pragma once

#include <map>
#include <memory>
#include <vector>

#include <boost/asio.hpp>

#include "../action.hpp"

namespace porla::Lua::Workflows::Actions
{
    struct HttpRequestOptions
    {
        sol::object                        body;
        boost::asio::io_context&           io;
        std::map<std::string, std::string> headers;
        sol::object                        method;
        sol::object                        url;
    };

class HttpRequest : public porla::Lua::Workflows::Action, public std::enable_shared_from_this<HttpRequest>
{
    public:
        explicit HttpRequest(HttpRequestOptions opts);
        ~HttpRequest();

        void Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback) override;

    private:
        HttpRequestOptions m_opts;
        std::unique_ptr<std::thread> m_thread;
    };
}
