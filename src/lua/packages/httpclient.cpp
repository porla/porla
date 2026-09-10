#include "httpclient.hpp"

#include <memory>
#include <stdexcept>
#include <string>

#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <curl/curl.h>

#include "../../curlmulti.hpp"
#include "../pluginstate.hpp"

using porla::Lua::Packages::HttpClient;

namespace
{
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

        std::string response_body;
        long        response_status = 0;

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
        auto*        state = static_cast<CurlTransferState*>(userdata);
        const size_t total = size * nmemb;

        state->response_body.append(ptr, total);

        return total;
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

    tbl.set_function("request", [](sol::this_state ts, std::string url, sol::protected_function callback, std::optional<sol::table> params)
    {
        sol::state_view lua(ts);

        auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
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
                            throw std::runtime_error(
                                "porla.http.request: header keys and values must be strings");
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
            throw std::runtime_error("curl_easy_init failed");
        }

        curl_easy_setopt(easy.get(), CURLOPT_URL,            url.c_str());
        curl_easy_setopt(easy.get(), CURLOPT_WRITEFUNCTION,  &CurlWriteCallback);
        curl_easy_setopt(easy.get(), CURLOPT_WRITEDATA,      transfer_state.get());
        curl_easy_setopt(easy.get(), CURLOPT_ERRORBUFFER,    transfer_state->error);
        curl_easy_setopt(easy.get(), CURLOPT_TIMEOUT,        static_cast<long>(timeout));
        curl_easy_setopt(easy.get(), CURLOPT_CONNECTTIMEOUT, static_cast<long>(connect_timeout));
        curl_easy_setopt(easy.get(), CURLOPT_FOLLOWLOCATION, follow_redirects ? 1L : 0L);
        curl_easy_setopt(easy.get(), CURLOPT_MAXREDIRS,      static_cast<long>(max_redirects));
        curl_easy_setopt(easy.get(), CURLOPT_NOSIGNAL, 1L);

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

        if (!transfer_state->request_body.empty())
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
                auto state = weak.lock();
                if (!state) { return; }

                transfer_state->result = result;

                curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &transfer_state->response_status);

                boost::asio::post(
                    state->io,
                    [state, transfer_state, callback_id]()
                    {
                        sol::table tbl = state->lua.create_table();
                        tbl["body"]   = transfer_state->response_body;
                        tbl["status"] = transfer_state->response_status;

                        state->InvokeCallback(callback_id, tbl);
                    });
            });
    });

    return tbl;
}
