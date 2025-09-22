#include "webui.hpp"

#include <fstream>
#include <sstream>

#include <boost/log/trivial.hpp>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include "buildinfo.hpp"

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

    if (curl_easy_perform(curl) != CURLE_OK)
    {
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

void WebUI::Download(const std::string& repo, const fs::path& target_file)
{
    std::stringstream url;
    url << "https://api.github.com/repos/" << repo << "/releases/latest";

    BOOST_LOG_TRIVIAL(info) << "Downloading web UI from github.com/" << repo << " to " << target_file;

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

    std::ofstream asset_output_file(target_file, std::ios::binary);
    asset_output_file << asset_data.body;
    asset_output_file.flush();
}
