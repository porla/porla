#include "../packages.hpp"

#include <algorithm>
#include <cctype>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <curl/curl.h>

#include "../pluginstate.hpp"
#include "../types/poerror.hpp"
#include "../../curlmulti.hpp"

using porla::Lua::Packages::HttpClient;
using porla::Lua::Types::PoError;
using porla::Lua::Error;

namespace Errors = porla::Errors;

namespace
{
    static constexpr std::size_t kMaxResponseBytes = 8 * 1024 * 1024;

    struct CurlTransferState
    {
        ~CurlTransferState()
        {
            // RAII: freed no matter which path we leave through, including a
            // throw during setup.
            if (request_header_list != nullptr)
            {
                curl_slist_free_all(request_header_list);
            }
        }

        std::string request_body;
        curl_slist* request_header_list = nullptr;

        std::string                        response_body;
        std::map<std::string, std::string> response_headers;
        long                               response_status = 0;
        std::string                        response_url;
        long                               response_redirects = 0;
        double                             response_time = 0;

        CURLcode result = CURLE_OK;
        char     error[CURL_ERROR_SIZE] = {};
    };

    struct EasyHandle
    {
        CURL* get() const { return handle; }
        CURL* release()   { CURL* h = handle; handle = nullptr; return h; }

        ~EasyHandle() { if (handle != nullptr) curl_easy_cleanup(handle); }

        CURL* handle = curl_easy_init();
    };

    size_t CurlWriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata)
    {
        const size_t total = size * nmemb;
        auto*        state = static_cast<CurlTransferState*>(userdata);

        if (state->response_body.size() + total > kMaxResponseBytes)
        {
            return 0;
        }

        state->response_body.append(ptr, total);

        return total;
    }

    size_t CurlHeaderCallback(char* ptr, size_t size, size_t nmemb, void* userdata)
    {
        const size_t     total = size * nmemb;
        auto*            state = static_cast<CurlTransferState*>(userdata);
        std::string_view line(ptr, total);

        // curl passes the headers of every redirect hop; each response starts with its status line,
        // so starting over there keeps only the final response's headers
        if (line.starts_with("HTTP/"))
        {
            state->response_headers.clear();
            return total;
        }

        const auto colon = line.find(':');

        if (colon == std::string_view::npos)
        {
            return total;
        }

        auto trim = [](std::string_view v)
        {
            while (!v.empty() && (v.front() == ' ' || v.front() == '\t')) v.remove_prefix(1);
            while (!v.empty() && (v.back() == ' ' || v.back() == '\t' || v.back() == '\r' || v.back() == '\n')) v.remove_suffix(1);
            return v;
        };

        std::string name(trim(line.substr(0, colon)));

        std::ranges::transform(
            name,
            name.begin(),
            [](unsigned char c)
            {
                return static_cast<char>(std::tolower(c));
            });

        const auto value = trim(line.substr(colon + 1));
        auto&      entry = state->response_headers[name];

        entry = entry.empty()
            ? std::string(value)
            : entry + ", " + std::string(value);

        return total;
    }

    std::string TransferErrorText(const CurlTransferState& state)
    {
        if (state.result == CURLE_WRITE_ERROR || state.result == CURLE_FILESIZE_EXCEEDED)
        {
            return "response exceeds the maximum size of "
                + std::to_string(kMaxResponseBytes) + " bytes";
        }

        if (state.error[0] != '\0')
        {
            return state.error;
        }

        return curl_easy_strerror(state.result);
    }

    sol::table TransferError(lua_State* L, const CurlTransferState& state)
    {
        sol::table err = PoError::Make(L, Errors::Code::http_error, TransferErrorText(state));
        err["category"] = "curl";
        err["value"]    = static_cast<int>(state.result);

        return err;
    }

    template<typename T>
    T GetOpt(const std::optional<sol::table>& tbl, const char* key, T default_value)
    {
        if (!tbl.has_value()) { return default_value; }
        return tbl->get_or(key, std::move(default_value));
    }
}

sol::object HttpClient::Load(sol::this_state ts)
{
    sol::state_view lua(ts);

    sol::table tbl = lua.create_table();

    const auto request = [](sol::this_state ts, std::string url, std::optional<sol::table> params, sol::protected_function callback)
    {
        sol::state_view lua(ts);

        auto weak = LuaState::WeakFrom(ts);
        auto state = weak.lock();

        if (state == nullptr)
        {
            return;
        }

        const auto method           = GetOpt<std::string>(params, "method", "GET");
        const auto body             = GetOpt<std::string>(params, "body", "");
        const auto timeout          = GetOpt<int>(params, "timeout", 30);
        const auto connect_timeout  = GetOpt<int>(params, "connect_timeout", 10);
        const auto follow_redirects = GetOpt<bool>(params, "follow_redirects", true);
        const auto max_redirects    = GetOpt<int>(params, "max_redirects", 10);

        if (timeout < 0 || connect_timeout < 0)
        {
            throw Error(Errors::Code::invalid_params, "http_client.request: timeout and connect_timeout must be >= 0");
        }

        auto transfer_state = std::make_shared<CurlTransferState>();

        if (params.has_value())
        {
            if (auto headers = params->get<sol::optional<sol::table>>("headers"))
            {
                headers->for_each(
                    [&transfer_state](sol::object key, sol::object val)
                    {
                        if (!key.is<std::string>() || !val.is<std::string>())
                        {
                            throw Error(Errors::Code::invalid_params,
                                "http_client.request: header keys and values must be strings");
                        }

                        const std::string header =
                            key.as<std::string>() + ": " + val.as<std::string>();

                        transfer_state->request_header_list = curl_slist_append(
                            transfer_state->request_header_list, header.c_str());
                    });
            }
        }

        transfer_state->request_body = body;

        EasyHandle easy;

        if (easy.get() == nullptr)
        {
            throw Error(Errors::Code::internal_error, "curl_easy_init failed");
        }

        curl_easy_setopt(easy.get(), CURLOPT_URL,                 url.c_str());
        curl_easy_setopt(easy.get(), CURLOPT_WRITEFUNCTION,       &CurlWriteCallback);
        curl_easy_setopt(easy.get(), CURLOPT_WRITEDATA,           transfer_state.get());
        curl_easy_setopt(easy.get(), CURLOPT_HEADERFUNCTION,      &CurlHeaderCallback);
        curl_easy_setopt(easy.get(), CURLOPT_HEADERDATA,          transfer_state.get());
        curl_easy_setopt(easy.get(), CURLOPT_ERRORBUFFER,         transfer_state->error);
        curl_easy_setopt(easy.get(), CURLOPT_TIMEOUT,             static_cast<long>(timeout));
        curl_easy_setopt(easy.get(), CURLOPT_CONNECTTIMEOUT,      static_cast<long>(connect_timeout));
        curl_easy_setopt(easy.get(), CURLOPT_FOLLOWLOCATION,      follow_redirects ? 1L : 0L);
        curl_easy_setopt(easy.get(), CURLOPT_MAXREDIRS,           static_cast<long>(max_redirects));
        curl_easy_setopt(easy.get(), CURLOPT_NOSIGNAL,            1L);
        curl_easy_setopt(easy.get(), CURLOPT_PROTOCOLS_STR,       "http,https");
        curl_easy_setopt(easy.get(), CURLOPT_REDIR_PROTOCOLS_STR, "http,https");
        curl_easy_setopt(easy.get(), CURLOPT_MAXFILESIZE_LARGE,   static_cast<curl_off_t>(kMaxResponseBytes));

        if (method == "POST")
        {
            curl_easy_setopt(easy.get(), CURLOPT_POST, 1L);
        }
        else if (method != "GET")
        {
            curl_easy_setopt(easy.get(), CURLOPT_CUSTOMREQUEST, method.c_str());

            if (method == "HEAD")
            {
                curl_easy_setopt(easy.get(), CURLOPT_NOBODY, 1L);
            }
        }

        // without post fields libcurl reads a POST body from stdin
        if (method == "POST" || !transfer_state->request_body.empty())
        {
            curl_easy_setopt(easy.get(), CURLOPT_POSTFIELDSIZE,
                static_cast<long>(transfer_state->request_body.size()));
            curl_easy_setopt(easy.get(), CURLOPT_COPYPOSTFIELDS,
                transfer_state->request_body.c_str());
        }

        if (transfer_state->request_header_list != nullptr)
        {
            curl_easy_setopt(easy.get(), CURLOPT_HTTPHEADER, transfer_state->request_header_list);
        }

        auto callback_id = state->RegisterCallback(callback, true);

        state->curl->AddTransfer(
            easy.release(),
            [weak, transfer_state, callback_id](CURL* easy, CURLcode result)
            {
                // the handle is cleaned up after this returns, and transfer_state may be gone by then
                curl_easy_setopt(easy, CURLOPT_ERRORBUFFER, nullptr);

                auto state = weak.lock();
                if (!state) { return; }

                transfer_state->result = result;

                curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &transfer_state->response_status);
                curl_easy_getinfo(easy, CURLINFO_REDIRECT_COUNT, &transfer_state->response_redirects);
                curl_easy_getinfo(easy, CURLINFO_TOTAL_TIME, &transfer_state->response_time);

                if (const char* url = nullptr; curl_easy_getinfo(easy, CURLINFO_EFFECTIVE_URL, &url) == CURLE_OK && url != nullptr)
                {
                    transfer_state->response_url = url;
                }

                boost::asio::post(
                    state->io,
                    [weak, transfer_state, callback_id]()
                    {
                        auto state = weak.lock();
                        if (!state) { return; }

                        if (transfer_state->result != CURLE_OK)
                        {
                            state->InvokeCallback(
                                callback_id,
                                TransferError(state->lua.lua_state(), *transfer_state),
                                sol::lua_nil);

                            return;
                        }

                        sol::table tbl   = state->lua.create_table();
                        tbl["body"]      = transfer_state->response_body;
                        tbl["headers"]   = sol::as_table(transfer_state->response_headers);
                        tbl["redirects"] = transfer_state->response_redirects;
                        tbl["status"]    = transfer_state->response_status;
                        tbl["time"]      = transfer_state->response_time;
                        tbl["url"]       = transfer_state->response_url;

                        state->InvokeCallback(callback_id, sol::lua_nil, tbl);
                    });
            });
    };

    tbl.set_function("request", sol::overload(
        [request](sol::this_state ts, std::string url, sol::protected_function callback)
        {
            request(ts, std::move(url), std::nullopt, std::move(callback));
        },
        [request](sol::this_state ts, std::string url, sol::table params, sol::protected_function callback)
        {
            request(ts, std::move(url), std::move(params), std::move(callback));
        }));

    return tbl;
}
