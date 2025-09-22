#include "webui.hpp"

#include <sstream>

#include <boost/log/trivial.hpp>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include "buildinfo.hpp"

using json = nlohmann::json;
using porla::WebUI;

static size_t HttpWriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    std::stringstream* ss = reinterpret_cast<std::stringstream*>(userdata);
    ss->write(ptr, nmemb);
    return nmemb;
}

void WebUI::Download(const std::string& repo, const fs::path& target_file)
{
    std::stringstream url;
    url << "https://api.github.com/repos/" << repo << "/releases/latest";

    std::stringstream user_agent;
    user_agent << "porla/" << BuildInfo::Version();

    BOOST_LOG_TRIVIAL(info) << "Downloading web UI from github.com/" << repo << " to " << target_file;

    std::stringstream http_response_body;

    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &http_response_body);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HttpWriteCallback);
    curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, user_agent.str().c_str());
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    json release = json::parse(http_response_body.str());
    std::string tag_name = release["tag_name"];
    std::string download_url = release["assets"][0]["browser_download_url"];

    BOOST_LOG_TRIVIAL(info) << "Found version " << tag_name << " of web UI - fetching from " << download_url;
}
