#include "webui.hpp"

#include <fstream>
#include <regex>
#include <sstream>

#include <boost/log/trivial.hpp>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include "buildinfo.hpp"
#include "curlmulti.hpp"
#include "data/models/keyvaluestore.hpp"
#include "utils/base64.hpp"
#include "zip.hpp"

using json = nlohmann::json;
using porla::WebUI;

const static std::map<std::string, std::string> MimeTypes =
{
    {".css", "text/css"},
    {".html", "text/html"},
    {".js", "text/javascript"},
    {".json", "application/json"},
    {".svg", "image/svg+xml"}
};

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

WebUI::WebUI(boost::asio::io_context& io, fs::path state_dir, sqlite3* db, std::weak_ptr<CurlMulti> cm)
    : m_io(io)
    , m_state_dir(state_dir)
    , m_db(db)
    , m_cm(cm)
{
}

std::shared_ptr<WebUI> WebUI::Create(boost::asio::io_context& io, fs::path state_dir, sqlite3* db, std::weak_ptr<CurlMulti> cm)
{
    auto webui = new WebUI(io, state_dir, db, cm);
    webui->LoadCurrent();

    return std::shared_ptr<WebUI>(webui);
}

bool WebUI::Has()
{
    const auto current_webui = Data::Models::KeyValueStore::Get(m_db, "porla.webui.current");
    return fs::exists(m_state_dir / "webui" / current_webui);
}

void WebUI::Install(const std::string& version, std::function<void()> callback)
{
    const auto owner_json      = Data::Models::KeyValueStore::Get(m_db, "porla.webui.owner");
    const auto repository_json = Data::Models::KeyValueStore::Get(m_db, "porla.webui.repository");

    const std::string owner = owner_json.is_null()
        ? "porla"
        : owner_json;

    const std::string repository = repository_json.is_null()
        ? "web"
        : repository_json;

    BOOST_LOG_TRIVIAL(info) << "Installing version " << version << " from GitHub repository " << owner << "/" << repository;

    std::stringstream url;
    url << "https://api.github.com/repos/" << owner << "/" << repository << "/releases/" << version;

    auto cm = m_cm.lock();

    if (cm == nullptr)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to lock CurlMulti";
        return;
    }

    cm->HttpGet(
        url.str(),
        [callback, owner, repository, w = weak_from_this()](const auto status, const auto body)
        {
            auto self = w.lock();

            if (!self)
            {
                BOOST_LOG_TRIVIAL(error) << "Failed to lock WebUI pointer";
                return;
            }

            const auto release = nlohmann::json::parse(body);

            BOOST_LOG_TRIVIAL(debug) << "GitHub release JSON: " << release;

            std::string tag_name = release["tag_name"];
            std::string download_url = release["assets"][0]["browser_download_url"];

            BOOST_LOG_TRIVIAL(info) << "Found version " << tag_name << " of web UI - fetching from " << download_url;

            auto cm = self->m_cm.lock();

            if (cm == nullptr)
            {
                return;
            }

            cm->HttpGet(
                download_url,
                [callback, owner, repository, tag_name, w](const auto status, const auto body)
                {
                    BOOST_LOG_TRIVIAL(info) << "Fetched " << body.size() << " bytes of fresh web UI";

                    auto self = w.lock();

                    if (!self)
                    {
                        BOOST_LOG_TRIVIAL(error) << "Self expired";
                        return;
                    }

                    const auto webui_dir = self->m_state_dir / "webui";

                    if (!fs::exists(webui_dir))
                    {
                        fs::create_directory(webui_dir);
                    }

                    std::stringstream webui_file_name;
                    webui_file_name << owner << "_" << repository << "_" << tag_name << ".zip";

                    {
                        std::ofstream out(webui_dir / webui_file_name.str(), std::ios::binary);
                        out << body;
                    }

                    Data::Models::KeyValueStore::Set(
                        self->m_db,
                        "porla.webui.current",
                        webui_file_name.str());

                    BOOST_LOG_TRIVIAL(info) << "Wrote web UI to " << webui_file_name.str();

                    boost::asio::post(self->m_io, [callback, w]()
                    {
                        auto s = w.lock();
                        if (!s) { return; }
                        s->LoadCurrent();

                        if (callback) callback();
                    });
                });
        });
}

std::function<void(uWS::HttpResponse<false>*, uWS::HttpRequest*)> WebUI::HttpHandler()
{
    return [weak = weak_from_this()](uWS::HttpResponse<false>* res, uWS::HttpRequest* req)
    {
        auto webui = weak.lock();

        if (!webui || webui->m_files.empty())
        {
            res->writeStatus("404 Not Found")->end("Nope");
            return;
        }

        auto const respond_with_file = [&res, &webui](const fs::path& file)
        {
            std::string mime_type = "text/plain";

            if (file.has_extension() && MimeTypes.contains(file.extension()))
            {
                mime_type = MimeTypes.at(file.extension());
            }

            if (!webui->m_files.contains(file))
            {
                res->writeStatus("404 Not found")->end("Not found");
                return;
            }

            std::string data = std::string(
                webui->m_files.at(file).data(),
                webui->m_files.at(file).size());

            if (file == "index.html")
            {
                str_replace_all(data, "%BASE_PATH%", webui->m_base_path);

                // Try to patch in our base path
                std::regex href_expression(R"(href=\"(\.\/)(.*)\")");
                std::regex src_expression(R"(src=\"(\.\/)(.*)\")");

                data = std::regex_replace(data, href_expression, "href=\"" + webui->m_base_path + "/$2\"");
                data = std::regex_replace(data, src_expression, "src=\"" + webui->m_base_path + "/$2\"");
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

        if (path.length() < webui->m_base_path.length()
            || path.substr(0, webui->m_base_path.length()) != webui->m_base_path)
        {
            res->writeStatus("404 Not found")->end("Not found");
            return;
        }

        std::string rooted_path = path.substr(webui->m_base_path.length());

        if (rooted_path.length() > 0 && rooted_path[0] == '/') rooted_path = rooted_path.substr(1);
        if (rooted_path.empty())                               rooted_path = "index.html";
        if (!webui->m_files.contains(rooted_path))             rooted_path = "index.html";

        respond_with_file(rooted_path);
    };
}

void WebUI::LoadCurrent()
{
    const auto current = Data::Models::KeyValueStore::Get(m_db, "porla.webui.current");
    const auto current_file = m_state_dir / "webui" / current;

    if (!fs::exists(current_file))
    {
        BOOST_LOG_TRIVIAL(info) << "Configured web UI file " << current_file << " does not exist";
        return;
    }

    std::ifstream file(current_file, std::ios::binary);

    const auto contents = std::vector<char>(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>());

    m_files.clear();
    m_files = Zip::Load(contents);

    BOOST_LOG_TRIVIAL(info) << "Web UI loaded from " << current_file;
}
