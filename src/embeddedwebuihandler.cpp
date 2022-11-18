#include "embeddedwebuihandler.hpp"

#include <filesystem>

#include <boost/log/trivial.hpp>
#include <zip.h>

namespace fs = std::filesystem;
using porla::EmbeddedWebUIHandler;

extern "C"
{
    extern const uint8_t webui_zip_data[];
    extern const unsigned webui_zip_size;
}

EmbeddedWebUIHandler::EmbeddedWebUIHandler()
{
    if (webui_zip_size == 0)
    {
        BOOST_LOG_TRIVIAL(info) << "No embedded web UI found";
    }
    else
    {
        BOOST_LOG_TRIVIAL(info) << "Loading embedded web UI (" << webui_zip_size/1024 << " kB)";

        zip_error_t err;
        zip_source_t *source = zip_source_buffer_create(webui_zip_data, webui_zip_size, 0, &err);

        zip_t *webui = zip_open_from_source(source, ZIP_RDONLY, &err);
        zip_int64_t num_entries = zip_get_num_entries(webui, ZIP_FL_UNCHANGED);

        for (int i = 0; i < num_entries; i++)
        {
            zip_stat_t st;
            zip_stat_init(&st);
            zip_stat_index(webui, i, 0, &st);

            if (st.size == 0)
            {
                continue;
            }

            zip_file* file = zip_fopen_index(webui, i, ZIP_FL_UNCHANGED);

            std::vector<char> buffer;
            buffer.resize(st.size);

            zip_fread(file, &buffer[0], st.size);
            zip_fclose(file);

            m_files.emplace(st.name, buffer);
        }

        zip_close(webui);
        zip_source_close(source);
    }
}

void EmbeddedWebUIHandler::operator()(const std::shared_ptr<HttpContext>& ctx)
{
    namespace http = boost::beast::http;

    const static std::map<std::string, std::string> MimeTypes =
    {
        {".css", "text/css"},
        {".html", "text/html"},
        {".js", "text/javascript"},
        {".svg", "image/svg+xml"}
    };

    auto const respond_with_file = [&ctx, this](const fs::path& file)
    {
        namespace http = boost::beast::http;

        std::string mime_type = "text/plain";

        if (file.has_extension() && MimeTypes.contains(file.extension()))
        {
            mime_type = MimeTypes.at(file.extension());
        }

        http::response<http::string_body> res{http::status::ok, ctx->Request().version()};
        res.set(http::field::server, "porla/1.0");
        res.set(http::field::content_type, mime_type);
        res.keep_alive(ctx->Request().keep_alive());
        res.body() = std::string(m_files.at(file).data(), m_files.at(file).size());
        res.prepare_payload();
        return res;
    };

    std::string path = ctx->Request().target().to_string();
    if (path.length() > 0 && path[0] == '/') path = path.substr(1);
    if (path.empty())                        path = "index.html";
    if (!m_files.contains(path))             path = "index.html";

    ctx->Write(respond_with_file(path));
}
