#include "webui.hpp"

#include <fstream>
#include <sstream>

#include <boost/log/trivial.hpp>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include "buildinfo.hpp"
#include "data/models/keyvaluestore.hpp"
#include "utils/base64.hpp"

using json = nlohmann::json;
using porla::WebUI;

struct HttpResponse
{
    std::string body;
    long status_code;
};

static size_t HttpWriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    std::stringstream* ss = reinterpret_cast<std::stringstream*>(userdata);
    ss->write(ptr, nmemb);
    return nmemb;
}

static HttpResponse HttpGet(const std::string& url)
{
    std::stringstream user_agent;
    user_agent << "porla/" << porla::BuildInfo::Version();

    std::stringstream http_response_body;

    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &http_response_body);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HttpWriteCallback);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, user_agent.str().c_str());

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK)
    {
        BOOST_LOG_TRIVIAL(error) << curl_easy_strerror(res);

        curl_easy_cleanup(curl);

        return HttpResponse{
            .body        = {},
            .status_code = -1
        };
    }

    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    curl_easy_cleanup(curl);

    return HttpResponse{
        .body        = http_response_body.str(),
        .status_code = response_code
    };
}

void WebUI::Download(sqlite3* db)
{
    const auto repo_json = Data::Models::KeyValueStore::Get(db, "porla.webui.repo");

    const std::string repo = repo_json.is_string()
        ? repo_json.get<std::string>()
        : "porla/web";

    std::stringstream url;
    url << "https://api.github.com/repos/" << repo << "/releases/latest";

    BOOST_LOG_TRIVIAL(info) << "Downloading web UI from GitHub repo " << repo;

    HttpResponse latest_release = HttpGet(url.str());

    if (latest_release.status_code != 200)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to fetch latest web UI release: " << latest_release.body;
        return;
    }

    json release;

    try
    {
        release = json::parse(latest_release.body);
    }
    catch (const std::exception& ex)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to parse release data: " << ex.what();
        return;
    }

    std::string tag_name = release["tag_name"];
    std::string download_url = release["assets"][0]["browser_download_url"];

    BOOST_LOG_TRIVIAL(info) << "Found version " << tag_name << " of web UI - fetching from " << download_url;

    HttpResponse asset_data = HttpGet(download_url);

    if (asset_data.status_code != 200)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to fetch web UI asset data: " << asset_data.body;
        return;
    }

    BOOST_LOG_TRIVIAL(info) << "Downloaded " << asset_data.body.size() << " bytes of fresh web UI";

    try
    {
        Data::Models::KeyValueStore::Set(db, "porla.webui.data", Utils::Base64::Encode(asset_data.body));
    }
    catch (const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to store web UI in key-value store: " << e.what();
    }
}
