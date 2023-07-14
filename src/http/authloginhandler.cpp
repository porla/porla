#include "authloginhandler.hpp"

#include <boost/log/trivial.hpp>
#include <jwt-cpp/jwt.h>
#include <nlohmann/json.hpp>
#include <sodium.h>

#include "../data/models/users.hpp"

using json = nlohmann::json;
using porla::Data::Models::Users;
using porla::Http::AuthLoginHandler;
using porla::Http::AuthLoginHandlerOptions;

struct AuthLoginHandler::State
{
    AuthLoginHandlerOptions  options;
    std::vector<std::thread> threads;
};

AuthLoginHandler::AuthLoginHandler(const AuthLoginHandlerOptions& opts)
{
    m_state = std::make_shared<State>(State{
        .options = opts,
        .threads = {}
    });
}

AuthLoginHandler::~AuthLoginHandler()
{
    for (auto& thread : m_state->threads)
    {
        if (thread.joinable()) thread.join();
    }
}

void AuthLoginHandler::operator()(uWS::HttpResponse<false>* res, uWS::HttpRequest* req)
{
    if (m_state->threads.size() >= 5)
    {
        return res->writeStatus("400 Bad Request")->end("Too many attempts");
    }

    res->onAborted([](){});

    std::string buffer;
    res->onData([res, state = m_state, buffer = std::move(buffer)](std::string_view d, bool last) mutable
    {
        buffer.append(d.data(), d.length());
        if (!last) return;

        nlohmann::json body;

        try
        {
            body = nlohmann::json::parse(buffer);
        }
        catch (const std::exception& err)
        {
            return res->writeStatus("400 Bad Request")->end(err.what());
        }

        if (!body.contains("username") || !body.contains("password"))
        {
            return res->writeStatus("400 Bad Request")->end("Invalid request");
        }

        const auto username = body["username"].get<std::string>();
        const auto password = body["password"].get<std::string>();

        const auto user = Users::GetByUsername(state->options.db, username);

        state->threads.emplace_back(
            [state, res, password, user]()
            {
                int result = -1;

                if (user)
                {
                    result = crypto_pwhash_str_verify(
                        user->password_hashed.c_str(),
                        password.c_str(),
                        password.size());
                }
                else
                {
                    // No user found. Still calculate a pwhash to not make it easy to
                    // enumerate usernames. Do not set the result. The hashed password is
                    // just 'hunter2'.

                    (void) crypto_pwhash_str_verify(
                        "$argon2id$v=19$m=1048576,t=4,p=1$MNy6/sqw4+WlGyeRDxiFdw$+FnYmB7Qfz+JKQeCzpjQW7rmpW/uqZxwGqDRDweBQRE",
                        "hunter2",
                        7);
                }

                boost::asio::dispatch(
                    state->options.io,
                    [state, res, result, thread_id = std::this_thread::get_id(), username = user->username]()
                    {
                        auto thread = std::find_if(
                            state->threads.begin(),
                            state->threads.end(),
                            [&thread_id](auto const& t) { return t.get_id() == thread_id; });

                        if (thread == state->threads.end())
                        {
                            // This really shouldn't happen.
                            BOOST_LOG_TRIVIAL(error) << "Could not find thread";
                        }
                        else
                        {
                            BOOST_LOG_TRIVIAL(debug) << "Erasing worker thread " << thread_id;

                            if (thread->joinable())
                            {
                                thread->join();
                            }

                            state->threads.erase(thread);
                        }

                        if (result != 0)
                        {
                            return res->end(json({
                                {"error", "invalid_auth"}
                            }).dump());
                        }

                        // Issue a JWT valid for 1 day. This should be enough for most users.
                        auto token = jwt::create()
                            .set_expires_at(std::chrono::system_clock::now() + std::chrono::days{1})
                            .set_issuer("porla")
                            .set_issued_at(std::chrono::system_clock::now())
                            .set_subject(username)
                            .set_type("JWS")
                            .sign(jwt::algorithm::hs256(state->options.secret_key));

                        res->end(json({
                            {"token", token}
                        }).dump());
                    });
            });
    });
}
