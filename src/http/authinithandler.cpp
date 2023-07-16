#include "authinithandler.hpp"

#include <thread>

#include <boost/log/trivial.hpp>
#include <nlohmann/json.hpp>
#include <sodium.h>

#include "../data/models/users.hpp"

using porla::Http::AuthInitHandler;

AuthInitHandler::AuthInitHandler(boost::asio::io_context& io, sqlite3* db, int memlimit)
    : m_io(io)
    , m_db(db)
    , m_memlimit(memlimit)
{
}

void AuthInitHandler::operator()(uWS::HttpResponse<false>* res, uWS::HttpRequest* req)
{
    if (porla::Data::Models::Users::Any(m_db))
    {
        return res->writeStatus("400 Bad Request")->end("Already initialized");
    }

    res->onAborted([](){});

    std::string buffer;
    res->onData([db = m_db, &io = m_io, res, buffer = std::move(buffer)](std::string_view d, bool last) mutable
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
            return res->writeStatus("400 Bad Request")->end("Missing 'username' or 'password'");
        }

        const auto username = body["username"].get<std::string>();
        const auto password = body["password"].get<std::string>();

        BOOST_LOG_TRIVIAL(info) << "Initializing auth with username " << username;

        std::thread t(
            [res, db, &io, username, password]()
            {
                std::string password_hashed;
                password_hashed.resize(crypto_pwhash_STRBYTES);

                int result = crypto_pwhash_str(
                    password_hashed.data(),
                    password.c_str(),
                    password.size(),
                    crypto_pwhash_OPSLIMIT_SENSITIVE,
                    crypto_pwhash_MEMLIMIT_SENSITIVE);

                boost::asio::dispatch(
                    io,
                    [res, db, password_hashed, result, username]()
                    {
                        if (result != 0)
                        {
                            BOOST_LOG_TRIVIAL(error) << "Out of memory when hashing password";
                            return;
                        }

                        if (porla::Data::Models::Users::Any(db))
                        {
                            BOOST_LOG_TRIVIAL(warning) << "A user was created while we where creating ours";
                            return;
                        }

                        porla::Data::Models::Users::Insert(
                            db,
                            porla::Data::Models::Users::User{
                                .username        = username,
                                .password_hashed = password_hashed,
                            });

                        BOOST_LOG_TRIVIAL(info) << "User " << username << " created";

                        res->writeStatus("200 OK")->end("{\"ok\": true }");
                    });
            });

        t.detach();
    });
}
