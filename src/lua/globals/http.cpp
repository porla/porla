#include "../globals.hpp"

#include <memory>
#include <stdexcept>
#include <string>

#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <curl/curl.h>

#include "../../curlmulti.hpp"

using porla::Lua::Globals::Http;

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

        // The suspended coroutine, plus a registry reference that keeps it from
        // being collected while the request is in flight.
        lua_State* thread     = nullptr;
        int        thread_ref = LUA_NOREF;
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
        auto*        td    = static_cast<CurlTransferState*>(userdata);
        const size_t total = size * nmemb;

        td->response_body.append(ptr, total);

        return total;
    }

    template<typename T>
    T GetOpt(const sol::table& tbl, const char* key, T default_value)
    {
        return tbl.get_or(key, std::move(default_value));
    }

    void ResumeCoroutine(const std::shared_ptr<CurlTransferState>& state)
    {
        lua_State* L = state->thread;

        if (L == nullptr)
        {
            return;
        }

        if (state->result == CURLE_OK)
        {
            sol::table res(L, sol::create);
            res["body"]   = state->response_body;
            res["status"] = state->response_status;

            sol::stack::push(L, res);
            sol::stack::push(L, sol::lua_nil);
        }
        else
        {
            const char* message = state->error[0] != '\0'
                ? state->error
                : curl_easy_strerror(state->result);

            sol::stack::push(L, sol::lua_nil);
            sol::stack::push(L, std::string(message));
        }

#if LUA_VERSION_NUM >= 504
        int nresults = 0;
        const int rc = lua_resume(L, nullptr, 2, &nresults);
#else
        const int rc = lua_resume(L, nullptr, 2);
#endif

        if (rc != LUA_OK && rc != LUA_YIELD)
        {
            const char* message = lua_tostring(L, -1);

            BOOST_LOG_TRIVIAL(error)
                << "porla.http: coroutine failed after request: "
                << (message != nullptr ? message : "unknown error");

            lua_pop(L, 1);
        }

        // Release the anchor. If the coroutine yielded again it will have taken
        // out a fresh reference of its own.
        luaL_unref(L, LUA_REGISTRYINDEX, state->thread_ref);

        state->thread_ref = LUA_NOREF;
        state->thread     = nullptr;
    }
}

sol::object Http::Build(sol::state& lua)
{
    sol::table http = lua.create_table();

    http["request"] = sol::yielding([](sol::this_state L, sol::table opts)
    {
        sol::state_view lua(L);

        if (!lua_isyieldable(L))
        {
            throw std::runtime_error("porla.http.request must be called from a coroutine");
        }

        auto multi = lua.registry()["curl"].get<std::shared_ptr<porla::CurlMulti>>();

        if (!multi)
        {
            throw std::runtime_error("porla.http: no curl instance registered");
        }

        auto state = std::make_shared<CurlTransferState>();

        // ------------------------------------------------------------------
        // Parse and validate everything before touching libcurl, so that a bad
        // argument raises a clean Lua error with nothing left dangling.
        // ------------------------------------------------------------------

        sol::optional<std::string> url = opts.get<sol::optional<std::string>>("url");

        if (!url || url->empty())
        {
            throw std::runtime_error("porla.http.request: 'url' is required");
        }

        const auto method           = GetOpt<std::string>(opts, "method", "GET");
        const auto body             = GetOpt<std::string>(opts, "body", "");
        const auto timeout          = GetOpt<int>(opts, "timeout", 30);
        const auto connect_timeout  = GetOpt<int>(opts, "connect_timeout", 10);
        const auto follow_redirects = GetOpt<bool>(opts, "follow_redirects", true);
        const auto max_redirects    = GetOpt<int>(opts, "max_redirects", 10);

        if (auto headers = opts.get<sol::optional<sol::table>>("headers"))
        {
            headers->for_each(
                [&state](sol::object key, sol::object val)
                {
                    if (!key.is<std::string>() || !val.is<std::string>())
                    {
                        throw std::runtime_error(
                            "porla.http.request: header keys and values must be strings");
                    }

                    const std::string header =
                        key.as<std::string>() + ": " + val.as<std::string>();

                    state->request_header_list =
                        curl_slist_append(state->request_header_list, header.c_str());
                });
        }

        state->request_body = body;

        // ------------------------------------------------------------------
        // Build the easy handle. EasyHandle cleans it up if anything below
        // throws; ownership moves to CurlMulti at the release() call.
        // ------------------------------------------------------------------

        EasyHandle easy;

        if (easy.get() == nullptr)
        {
            throw std::runtime_error("curl_easy_init failed");
        }

        curl_easy_setopt(easy.get(), CURLOPT_URL,            url->c_str());
        curl_easy_setopt(easy.get(), CURLOPT_WRITEFUNCTION,  &CurlWriteCallback);
        curl_easy_setopt(easy.get(), CURLOPT_WRITEDATA,      state.get());
        curl_easy_setopt(easy.get(), CURLOPT_ERRORBUFFER,    state->error);
        curl_easy_setopt(easy.get(), CURLOPT_TIMEOUT,        static_cast<long>(timeout));
        curl_easy_setopt(easy.get(), CURLOPT_CONNECTTIMEOUT, static_cast<long>(connect_timeout));
        curl_easy_setopt(easy.get(), CURLOPT_FOLLOWLOCATION, follow_redirects ? 1L : 0L);
        curl_easy_setopt(easy.get(), CURLOPT_MAXREDIRS,      static_cast<long>(max_redirects));

        // Without this libcurl uses signals for DNS timeouts, which is unsafe
        // in a threaded process.
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

        if (!state->request_body.empty())
        {
            // COPYPOSTFIELDS, not POSTFIELDS: libcurl takes its own copy, so we
            // don't have to reason about the buffer outliving the transfer.
            curl_easy_setopt(easy.get(), CURLOPT_POSTFIELDSIZE,
                static_cast<long>(state->request_body.size()));
            curl_easy_setopt(easy.get(), CURLOPT_COPYPOSTFIELDS,
                state->request_body.c_str());
        }

        if (state->request_header_list != nullptr)
        {
            curl_easy_setopt(easy.get(), CURLOPT_HTTPHEADER, state->request_header_list);
        }

        // ------------------------------------------------------------------
        // Anchor the coroutine so it can't be collected while suspended, then
        // hand the transfer over.
        // ------------------------------------------------------------------

        lua_pushthread(L);
        state->thread_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        state->thread     = L;

        auto executor = multi->Executor();

        multi->AddTransfer(
            easy.release(),
            [state, executor](CURL* easy, CURLcode result)
            {
                state->result = result;

                curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &state->response_status);

                // Never resume inline. AddTransfer() can invoke this callback
                // synchronously (add_handle failure, or a shut-down CurlMulti),
                // and at that point the coroutine has not yielded yet. Posting
                // guarantees the yield has happened first.
                boost::asio::post(
                    executor,
                    [state]() { ResumeCoroutine(state); });
            });

        // sol::yielding suspends here. The values pushed by ResumeCoroutine
        // become the return values of http.request().
    });

    return http;
}