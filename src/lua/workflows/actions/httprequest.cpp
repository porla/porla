#include "httprequest.hpp"

#include <map>
#include <thread>

#include <boost/log/trivial.hpp>
#include <curl/curl.h>
#include <utility>

#define RESOLVE_STRING(output, input, context)       \
    if (input.is<sol::function>())                   \
    {                                                \
        output = input.as<sol::function>()(context); \
    }                                                \
    else                                             \
    {                                                \
        output = input.as<std::string>();            \
    }

using porla::Lua::Workflows::ActionCallback;
using porla::Lua::Workflows::Actions::HttpRequest;
using porla::Lua::Workflows::Actions::HttpRequestOptions;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* user)
{
    auto buffer = reinterpret_cast<std::string*>(user);
    buffer->append(reinterpret_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

HttpRequest::HttpRequest(HttpRequestOptions opts)
    : m_opts(std::move(opts))
{
}

HttpRequest::~HttpRequest()
{
    if (m_thread && m_thread->joinable()) m_thread->join();
}

void HttpRequest::Invoke(const ActionParams& params, std::shared_ptr<ActionCallback> callback)
{
    struct ReqInput
    {
        std::string url{};
        std::string body{};
        std::string method{};
    };

    ReqInput input{};

    RESOLVE_STRING(input.url, m_opts.url, params.context)
    RESOLVE_STRING(input.body, m_opts.body, params.context)
    RESOLVE_STRING(input.method, m_opts.method, params.context)

    m_thread = std::make_unique<std::thread>(
        [_this = shared_from_this(), input, params, callback]()
        {
            CURL* curl = curl_easy_init();

            if (curl == nullptr)
            {
                boost::asio::dispatch(
                    [_this, callback]()
                    {
                        BOOST_LOG_TRIVIAL(error) << "(actions/http-request) Failed to init CURL";
                        callback->Complete();
                    });

                return;
            }

            std::string buffer;
            long response_code = -1;

            curl_slist* headers_list = nullptr;

            for (const auto& header : _this->m_opts.headers)
            {
                std::stringstream hdr;
                hdr << header.first << ": " << header.second;

                headers_list = curl_slist_append(headers_list, hdr.str().c_str());
            }

            curl_easy_setopt(curl, CURLOPT_URL, input.url.c_str());

            if (!input.body.empty())
            {
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, input.body.c_str());
            }

            if (!input.method.empty())
            {
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, input.method.c_str());
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
                BOOST_LOG_TRIVIAL(error) << "(actions/http-request) CURL error code: " << res;
            }

            curl_slist_free_all(headers_list);
            curl_easy_cleanup(curl);

            boost::asio::dispatch(
                _this->m_opts.io,
                [_this, callback, params, buffer, response_code]()
                {
                    sol::table output = params.state.create_table();
                    output["content"] = buffer;

                    if (response_code > -1)
                    {
                        output["status_code"] = response_code;
                    }

                    callback->Complete(output);
                });
        });
}
