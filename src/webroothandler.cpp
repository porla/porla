#include "webroothandler.hpp"

#include <filesystem>
#include <utility>

namespace fs = std::filesystem;
using porla::WebRootHandler;

WebRootHandler::WebRootHandler(std::string root)
    : m_root(std::move(root))
{
}

void WebRootHandler::operator()(const std::shared_ptr<porla::HttpContext> &ctx)
{
    static std::map<std::string, std::string> MimeTypes =
    {
        {".css",  "text/css"},
        {".html", "text/html"},
        {".js", "text/javascript"},
        {".svg", "image/svg+xml"}
    };

    if (ctx->Request().method() == boost::beast::http::verb::get)
    {
        fs::path path = fs::path(m_root).concat(ctx->Request().target().to_string());
        if(ctx->Request().target().back() == '/') { path.append("index.html"); }

        if (!fs::exists(path) || !fs::is_regular_file(path))
        {
            return ctx->Next();
        }

        boost::beast::error_code ec;
        boost::beast::http::file_body::value_type body;
        body.open(path.c_str(), boost::beast::file_mode::scan, ec);

        auto const size = body.size();

        std::string contentType = "text/plain";

        if (path.has_extension() && MimeTypes.find(path.extension().string()) != MimeTypes.end())
        {
            contentType = MimeTypes.at(path.extension().string());
        }

        boost::beast::http::response<boost::beast::http::file_body> res{
            std::piecewise_construct,
            std::make_tuple(std::move(body)),
            std::make_tuple(boost::beast::http::status::ok, ctx->Request().version())};
        res.set(boost::beast::http::field::server, "porla/1.0");
        res.set(boost::beast::http::field::cache_control, "max-age=10");
        res.set(boost::beast::http::field::content_type, contentType);
        res.content_length(size);
        res.keep_alive(ctx->Request().keep_alive());

        ctx->Write(std::move(res));
    }
    else
    {
        ctx->Next();
    }
}
