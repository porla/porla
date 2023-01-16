#pragma once

#include <functional>

#include <boost/asio.hpp>

namespace porla
{
    class HttpClient
    {
    public:
        struct Request
        {
            std::string url;
            std::string method;
            std::string body;
        };

        explicit HttpClient(boost::asio::io_context& io);
        void SendAsync(const Request& req, const std::function<void()>& callback);

    private:
        struct RequestState;

        boost::asio::io_context& m_io;
    };
}