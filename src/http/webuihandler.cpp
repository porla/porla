#include "webuihandler.hpp"

#include <filesystem>
#include <regex>

#include <boost/log/trivial.hpp>
#include <utility>
#include <zip.h>

namespace fs = std::filesystem;
using porla::Http::WebUIHandler;

extern "C"
{
    extern const uint8_t* webui_zip_data();
    extern const size_t webui_zip_size();
}


static void str_replace_all(std::string& str, const std::string& from, const std::string& to)
{
    if (from.empty())
    {
        return;
    }

    size_t start_pos = 0;

    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

WebUIHandler::WebUIHandler(std::string base_path)
    : m_base_path(std::move(base_path))
{
    if (webui_zip_size() == 0)
    {
        BOOST_LOG_TRIVIAL(warning) << "No embedded web UI found";
    }
    else
    {
        BOOST_LOG_TRIVIAL(debug) << "Loading embedded web UI (" << webui_zip_size()/1024 << " kB)";

        zip_error_t err;
        zip_source_t *source = zip_source_buffer_create(webui_zip_data(), webui_zip_size(), 0, &err);

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

void WebUIHandler::operator()(uWS::HttpResponse<false>* res, uWS::HttpRequest* req)
{
    // If files are empty (we have no embedded web UI) - return 404.
    if (m_files.empty())
    {
        res->writeStatus("404 Not found")->end("Not found");
        return;
    }

    const static std::map<std::string, std::string> MimeTypes =
    {
        {".css", "text/css"},
        {".html", "text/html"},
        {".js", "text/javascript"},
        {".svg", "image/svg+xml"}
    };

    auto const respond_with_file = [&res, this](const fs::path& file)
    {
        std::string mime_type = "text/plain";

        if (file.has_extension() && MimeTypes.contains(file.extension()))
        {
            mime_type = MimeTypes.at(file.extension());
        }

        std::string data = std::string(m_files.at(file).data(), m_files.at(file).size());

        if (file == "index.html")
        {
            str_replace_all(data, "%BASE_PATH%", m_base_path);

            // Try to patch in our base path
            std::regex href_expression(R"(href=\"(\.\/)(.*)\")");
            std::regex src_expression(R"(src=\"(\.\/)(.*)\")");

            data = std::regex_replace(data, href_expression, "href=\"" + m_base_path + "/$2\"");
            data = std::regex_replace(data, src_expression, "src=\"" + m_base_path + "/$2\"");
        }

        res->writeHeader("Content-Type", mime_type);
        res->writeStatus("200 OK");
        res->write(data);
        res->end();
    };

    std::string path = std::string(req->getUrl());

    // If the path is shorter than our base path, do not handle this request.
    // For example,
    // path: /      base_path: /porla
    // Also check that the path is prefixed with the base path.

    if (path.length() < m_base_path.length() || path.substr(0, m_base_path.length()) != m_base_path)
    {
        res->writeStatus("404 Not found")->end("Not found");
        return;
    }

    std::string rooted_path = path.substr(m_base_path.length());

    if (rooted_path.length() > 0 && rooted_path[0] == '/') rooted_path = rooted_path.substr(1);
    if (rooted_path.empty())                               rooted_path = "index.html";
    if (!m_files.contains(rooted_path))                    rooted_path = "index.html";

    respond_with_file(rooted_path);
}
