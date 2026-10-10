#pragma once

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

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

        using Record = Data::Models::Sessions::Session;

        // these are all the loaded sessions
        std::map<int, std::shared_ptr<Session>> All() { return m_sessions; }
        std::shared_ptr<Session> Get(int id);

        // query stored sessions, loaded or not
        std::optional<Record> Find(int id) const;
        std::optional<Record> FindByName(const std::string& name) const;
        std::optional<Record> FindDefault() const;
        std::vector<Record> List() const;

        // the id of the given session, or of the default session if no id is given
        // nullopt if that session does not exist
        std::optional<int> ResolveId(std::optional<int> id) const;

        int Add(const Record& session);
        void Remove(int id);
        void Update(const Record& session);

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
