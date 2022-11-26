#pragma once

#include <boost/beast.hpp>
#include <nlohmann/json.hpp>

namespace porla
{
    class HttpContext
    {
    public:
        struct Uri
        {
            std::string path;
        };

        virtual void Next() = 0;

        virtual boost::beast::http::request<boost::beast::http::string_body>& Request() = 0;
        virtual Uri& RequestUri() = 0;
        virtual boost::beast::tcp_stream& Stream() = 0;

        virtual void Write(std::string body) = 0;
        virtual void Write(boost::beast::http::response<boost::beast::http::file_body> res) = 0;
        virtual void Write(boost::beast::http::response<boost::beast::http::string_body> res) = 0;
        virtual void WriteJson(const nlohmann::json& j) = 0;
    };
}
