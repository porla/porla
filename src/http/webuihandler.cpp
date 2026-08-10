#include "webuihandler.hpp"

#include <filesystem>
#include <fstream>
#include <regex>

#include <boost/log/trivial.hpp>
#include <utility>

#include "../data/models/keyvaluestore.hpp"
#include "../utils/base64.hpp"
#include "../zip.hpp"

namespace fs = std::filesystem;
using porla::Http::WebUIHandler;

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

WebUIHandler::WebUIHandler(sqlite3* db, std::string base_path, boost::signals2::signal<void(const std::unordered_set<std::string>&)>& reload_signal)
    : m_db(db)
    , m_base_path(std::move(base_path))
{
    LoadUI();

    m_reload_connection = reload_signal.connect(
        [this](const std::unordered_set<std::string>& keys)
        {
            if (keys.contains("porla.webui.data"))
            {
                this->LoadUI();
            }
        });
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
        {".json", "application/json"},
        {".svg", "image/svg+xml"}
    };

    auto const respond_with_file = [&res, this](const fs::path& file)
    {
        std::string mime_type = "text/plain";

        if (file.has_extension() && MimeTypes.contains(file.extension()))
        {
            mime_type = MimeTypes.at(file.extension());
        }

        if (!m_files.contains(file))
        {
            res->writeStatus("404 Not found")->end("Not found");
            return;
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

void WebUIHandler::LoadUI()
{
    const auto webui_data = Data::Models::KeyValueStore::Get(m_db, "porla.webui.data");

    if (webui_data == nlohmann::json() || !webui_data.is_string())
    {
        return;
    }

    const auto decoded = Utils::Base64::Decode(webui_data.get<std::string>());

    BOOST_LOG_TRIVIAL(info) << "Loading web UI (" << decoded.size() << " bytes)";

    m_files.clear();
    m_files = Zip::Load(std::vector<char>(decoded.begin(), decoded.end()));
}

