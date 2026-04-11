#include "../types.hpp"

#include "../registry.hpp"
#include "../../curlmulti.hpp"

#include <curl/curl.h>

using porla::Lua::Types::Libcurl;

template<typename T>
T get_opt(sol::table& tbl, const char* key, T default_value)
{
    auto val = tbl[key];
    return val.valid() ? val.get<T>() : default_value;
}

struct CurlTransferState
{
    std::string request_body;
    curl_slist* request_header_list = nullptr;

    std::string response_body;
    long        response_status_code = 0;
};

static size_t CurlWriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    auto* td = static_cast<CurlTransferState*>(userdata);
    td->response_body.append(ptr, size * nmemb);
    return size * nmemb;
}

void Libcurl::Register(sol::state& lua)
{
    sol::table crl = lua["libcurl"].valid()
        ? lua["libcurl"].get<sol::table>()
        : lua.create_named_table("libcurl");

    crl["request"] = sol::yielding([](sol::this_state L, sol::table opts)
    {
        sol::state_view lua(L);

        auto multi = lua.registry()["curl"].get<CurlMulti*>();
        auto state = std::make_shared<CurlTransferState>();

        std::string url       = opts.get<std::string>("url");
        std::string method    = get_opt<std::string>(opts, "method", "GET");
        std::string body      = get_opt<std::string>(opts, "body", "");
        int timeout           = get_opt<int>(opts, "timeout", 30);
        bool follow_redirects = get_opt<bool>(opts, "follow_redirects", true);
        int max_redirects     = get_opt<int>(opts, "max_redirects", 10);

        CURL* easy = curl_easy_init();

        if (easy == nullptr)
        {
            sol::stack::push(L, sol::lua_nil);
            sol::stack::push(L, std::string("curl_easy_init failed"));
            return sol::lua_nil;
        }

        curl_easy_setopt(easy, CURLOPT_URL,            url.c_str());
        curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION,  CurlWriteCallback);
        curl_easy_setopt(easy, CURLOPT_WRITEDATA,      state.get());
        curl_easy_setopt(easy, CURLOPT_TIMEOUT,        static_cast<long>(timeout));
        curl_easy_setopt(easy, CURLOPT_FOLLOWLOCATION, follow_redirects ? 1L : 0L);
        curl_easy_setopt(easy, CURLOPT_MAXREDIRS,      static_cast<long>(max_redirects));

        if (method == "POST")
        {
            curl_easy_setopt(easy, CURLOPT_POST, 1L);
        }
        else
        {
            curl_easy_setopt(easy, CURLOPT_CUSTOMREQUEST, method.c_str());

            if (method == "HEAD")
            {
                curl_easy_setopt(easy, CURLOPT_NOBODY, 1L);
            }
        }

        if (!body.empty())
        {
            state->request_body = body;

            curl_easy_setopt(easy, CURLOPT_POSTFIELDS,    state->request_body.c_str());
            curl_easy_setopt(easy, CURLOPT_POSTFIELDSIZE, static_cast<long>(state->request_body.size()));
        }

        sol::optional<sol::table> headers = opts.get<sol::optional<sol::table>>("headers");

        if (headers)
        {
            headers->for_each(
                [&state](sol::object key, sol::object val)
                {
                    std::string header = key.as<std::string>() + ": " + val.as<std::string>();

                    state->request_header_list = curl_slist_append(
                        state->request_header_list, header.c_str());
                });

            curl_easy_setopt(easy, CURLOPT_HTTPHEADER, state->request_header_list);
        }

        multi->AddTransfer(easy, [state, L](CURL* easy)
        {
            if (state->request_header_list)
            {
                curl_slist_free_all(state->request_header_list);
            }

            long status = 0;
            curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &status);

            sol::table res(L, sol::create);
            res["body"]   = state->response_body;
            res["status"] = status;

            sol::stack::push(L, res);
            sol::stack::push(L, sol::lua_nil);
            
            int nresults = 0;
            lua_resume(L, nullptr, 2, &nresults);
        });

        return sol::lua_nil;
    });
}