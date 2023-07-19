#include "httpclient.hpp"

#include <map>
#include <thread>

#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <curl/curl.h>

#include "../plugins/plugin.hpp"

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* user)
{
    auto buffer = reinterpret_cast<std::string*>(user);
    buffer->append(reinterpret_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

using porla::Lua::Packages::HttpClient;

struct HttpRequestState
{
    std::string body;
    sol::function callback;
    std::map<std::string, std::string> headers;
    boost::asio::io_context& io;
    std::string method;
    std::string url;
    std::shared_ptr<std::thread> worker;
};

void HttpClient::Register(sol::state& lua)
{
    lua["package"]["preload"]["http.client"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        sol::table client = lua.create_table();

        client["request"] = [](sol::this_state s, const sol::table& args)
        {
            sol::state_view lua{s};
            const auto options = lua.globals()["__load_opts"].get<const Plugins::PluginLoadOptions&>();

            auto state = std::make_shared<HttpRequestState>(HttpRequestState{
                .body     = args["body"],
                .callback = args["callback"],
                .headers  = args["headers"].valid()
                    ? args["headers"].get<std::map<std::string, std::string>>()
                    : std::map<std::string, std::string>(),
                .io       = options.io,
                .method   = args["method"],
                .url      = args["url"],
                .worker   = nullptr
            });

            state->worker = std::make_shared<std::thread>(
                [s = state]()
                {
                    CURL* curl = curl_easy_init();

                    std::string buffer;
                    long response_code = -1;

                    curl_slist* headers_list = nullptr;

                    for (const auto& header : s->headers)
                    {
                        std::stringstream hdr;
                        hdr << header.first << ": " << header.second;

                        headers_list = curl_slist_append(headers_list, hdr.str().c_str());
                    }

                    curl_easy_setopt(curl, CURLOPT_URL, s->url.c_str());

                    if (!s->body.empty())
                    {
                        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, s->body.c_str());
                    }

                    if (!s->method.empty())
                    {
                        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, s->method.c_str());
                    }

                    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers_list);
                    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

                    CURLcode res = curl_easy_perform(curl);

                    if (res == CURLE_OK)
                    {
                        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
                    }
                    else
                    {
                        BOOST_LOG_TRIVIAL(error) << "CURL error code: " << res;
                    }

                    curl_slist_free_all(headers_list);
                    curl_easy_cleanup(curl);

                    boost::asio::dispatch(
                        s->io,
                        [s, buffer, response_code]()
                        {
                            if (s->worker->joinable())
                            {
                                s->worker->join();
                            }

                            s->worker = nullptr;

                            s->callback(response_code, buffer);
                        });
                });
        };

        return client;
    };
}
