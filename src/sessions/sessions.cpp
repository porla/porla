#include "sessions.hpp"

#include <boost/log/trivial.hpp>

#include "session.hpp"
#include "sessionevent.hpp"

#include "../events.hpp"

using porla::Session;
using porla::Sessions;

Sessions::Sessions(const SessionsOptions& options)
    : m_options(options)
{
}

Sessions::~Sessions()
{
    std::vector<std::shared_ptr<Session>> pending;
    pending.reserve(m_sessions.size());

    BOOST_LOG_TRIVIAL(info) << "Shutting down " << pending.size() << " session(s)";

    while (!m_sessions.empty())
    {
        auto node    = m_sessions.extract(m_sessions.begin());
        auto session = std::move(node.mapped());

        try
        {
            session->Stop();
        }
        catch(const std::exception& e)
        {
            BOOST_LOG_TRIVIAL(error) << "session[" << session->Name() << "] Failed to unload session: " << e.what();
        }

        pending.push_back(std::move(session));
    }

    BOOST_LOG_TRIVIAL(info) << "All state saved";

    for (auto& session : pending)
    {
        const auto name = session->Name();

        BOOST_LOG_TRIVIAL(info)
            << "session[" << name << "] Destroying session - this is safe to interrupt";

        std::weak_ptr<Session> observer = session;

        session.reset();

        if (!observer.expired())
        {
            BOOST_LOG_TRIVIAL(warning)
                << "session[" << name << "] Still referenced after unload - "
                << "destruction deferred past shutdown";
        }
    }
}

std::shared_ptr<Session> Sessions::Get(int id)
{
    const auto it = m_sessions.find(id);

    return it == m_sessions.end()
        ? nullptr
        : it->second;
}

int Sessions::Add(const Data::Models::Sessions::Session& session)
{
    const int id = Data::Models::Sessions::Insert(m_options.db, session);

    BOOST_LOG_TRIVIAL(info) << "session[" << session.name << "] Added - loading";

    LoadById(id);

    if (const auto session = Get(id))
    {
        SessionEvent added("session.added");
        added.session_id = session->Id();
        added.session    = session;

        m_options.events.Publish(std::move(added));
    }

    return id;
}

void Sessions::Remove(int id)
{
    const auto session = Data::Models::Sessions::GetById(m_options.db, id);

    if (!session)
    {
        return;
    }

    UnloadById(id);

    Data::Models::Sessions::Remove(m_options.db, id);

    BOOST_LOG_TRIVIAL(info) << "session[" << session->name << "] Removed";

    PublishDetached(id, SessionEvent("session.removed", {{ "session_name", session->name }}));
}

void Sessions::Update(const Data::Models::Sessions::Session& session)
{
    Data::Models::Sessions::Update(m_options.db, session);

    if (const auto instance = Get(session.id))
    {
        instance->Rename(session.name);

        SessionEvent updated("session.updated");
        updated.session_id = instance->Id();
        updated.session    = instance;

        m_options.events.Publish(std::move(updated));
    }
}


void Sessions::Load(const std::function<void()>& load_callback)
{
    const auto sessions = Data::Models::Sessions::List(m_options.db);

    BOOST_LOG_TRIVIAL(info) << "Loading " << sessions.size() << " session(s)";

    if (sessions.empty())
    {
        if (load_callback)
        {
            boost::asio::post(m_options.io, load_callback);
        }

        return;
    }

    // All sessions are loaded concurrently. After each one has loaded, this
    // counter is decreased by one. When it hits zero, the callback is invoked.
    auto outstanding_sessions = std::make_shared<std::size_t>(sessions.size());

    auto single_session_loaded = [this, outstanding_sessions, load_callback]()
    {
        --(*outstanding_sessions);

        if (*outstanding_sessions > 0)
        {
            return;
        }

        if (load_callback)
        {
            boost::asio::post(m_options.io, load_callback);
        }
    };

    for (const auto& session : sessions)
    {
        LoadById(session.id, single_session_loaded);
    }
}

void Sessions::LoadById(int id, const std::function<void()>& load_callback)
{
    const auto record = Data::Models::Sessions::GetById(m_options.db, id);

    if (!record)
    {
        BOOST_LOG_TRIVIAL(warning) << "No session with ID " << id;
        if (load_callback) { boost::asio::post(m_options.io, load_callback); }
        return;
    }

    if (m_sessions.contains(record->id))
    {
        BOOST_LOG_TRIVIAL(warning) << "session[" << record->name << "] Already loaded - skipping";
        if (load_callback) { boost::asio::post(m_options.io, load_callback); }
        return;
    }

    auto session = std::make_shared<Session>(SessionOptions{
        .record = *record,
        .db     = m_options.db,
        .events = m_options.events,
        .io     = m_options.io
    });

    m_sessions.insert({ session->Id(), session });

    session->Start(load_callback);
}

void Sessions::UnloadById(int id)
{
    const auto it = m_sessions.find(id);

    if (it == m_sessions.end())
    {
        return;
    }

    const auto name = it->second->Name();

    it->second->Stop();

    m_sessions.erase(it);

    PublishDetached(id, SessionEvent("session.unloaded", {{ "session_name", name }}));
}

template <typename T>
void Sessions::PublishDetached(int session_id, T event)
{
    static_assert(std::is_base_of_v<SessionEvent, T>);

    event.session_id = session_id;

    m_options.events.Publish(std::move(event));
}
