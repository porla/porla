#pragma once

#include <functional>
#include <map>
#include <memory>

#include <boost/asio/io_context.hpp>
#include <sqlite3.h>

#include "../data/models/sessions.hpp"

namespace porla
{
    class Events;
    class Session;

    struct SessionsOptions
    {
        sqlite3*                 db;
        Events&                  events;
        boost::asio::io_context& io;
    };

    // manages all sessions and the session.* events
    class Sessions
    {
    public:
        explicit Sessions(const SessionsOptions& options);
        ~Sessions();

        Sessions(const Sessions&)            = delete;
        Sessions& operator=(const Sessions&) = delete;

        std::map<int, std::shared_ptr<Session>> All() { return m_sessions; }
        std::shared_ptr<Session> Get(int id);

        int Add(const Data::Models::Sessions::Session& session);
        void Remove(int id);
        void Update(const Data::Models::Sessions::Session& session);

        void Load(const std::function<void()>& load_callback = {});
        void LoadById(int id, const std::function<void()>& load_callback = {});
        void UnloadById(int id);

    private:
        template<typename T>
        void PublishDetached(int session_id, T event);

        SessionsOptions                         m_options;
        std::map<int, std::shared_ptr<Session>> m_sessions;
    };
}
