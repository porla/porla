#include "sessions.hpp"

#include "session.hpp"
#include "sessionevent.hpp"

#include "../data/models/addtorrentparams.hpp"
#include "../data/models/presets.hpp"
#include "../data/transaction.hpp"
#include "../events.hpp"
#include "../logger.hpp"

using porla::Data::Transaction;
using porla::Session;
using porla::Sessions;

namespace
{
    BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(logger, porla::Log::Logger) { return porla::Log::Component("session"); }
}

Sessions::Sessions(const SessionsOptions& options)
    : m_options(options)
{
}

Sessions::~Sessions()
{
    std::vector<std::shared_ptr<Session>> pending;
    pending.reserve(m_sessions.size());

    BOOST_LOG_SEV(logger::get(), Log::info)
        << "Shutting down " << pending.size() << " session(s)";

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
            BOOST_LOG_SEV(logger::get(), Log::error)
                << Log::Property("error", e.what())
                << Log::Property("session_id", session->Id())
                << Log::Property("session_name", session->Name())
                << "Failed to unload session";
        }

        pending.push_back(std::move(session));
    }

    BOOST_LOG_SEV(logger::get(), Log::info) << "All state saved";

    for (auto& session : pending)
    {
        const auto name = session->Name();

        BOOST_LOG_SEV(logger::get(), Log::info)
            << Log::Property("session_id", session->Id())
            << Log::Property("session_name", session->Name())
            << "Destroying session - this is safe to interrupt";

        std::weak_ptr<Session> observer = session;

        session.reset();

        if (!observer.expired())
        {
            BOOST_LOG_SEV(logger::get(), Log::warning)
                << Log::Property("session_id", session->Id())
                << Log::Property("session_name", session->Name())
                << "Still referenced after unload - destruction deferred past shutdown";
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

std::size_t Sessions::CountParams(int id) const
{
    return Data::Models::AddTorrentParams::Count(m_options.db, id);
}

void Sessions::MoveParams(int from_id, int to_id, const lt::info_hash_t& hash, const lt::add_torrent_params& params, const TorrentClientData& client_data)
{
    // one transaction, so the torrent is stored in exactly one of the sessions at every point
    Transaction tx(m_options.db);

    Data::Models::AddTorrentParams::Move(m_options.db, from_id, to_id, hash, params, client_data);

    if (!tx.Commit())
    {
        throw std::runtime_error("Failed to commit torrent move");
    }
}

std::optional<Sessions::Record> Sessions::Find(int id) const
{
    return Data::Models::Sessions::GetById(m_options.db, id);
}

std::optional<Sessions::Record> Sessions::FindByName(const std::string& name) const
{
    return Data::Models::Sessions::GetByName(m_options.db, name);
}

std::optional<Sessions::Record> Sessions::FindDefault() const
{
    return Data::Models::Sessions::GetDefault(m_options.db);
}

std::vector<Sessions::Record> Sessions::List() const
{
    return Data::Models::Sessions::List(m_options.db);
}

std::optional<Sessions::Record> Sessions::Resolve(std::optional<int> id) const
{
    return id.has_value() ? Find(*id) : FindDefault();
}

int Sessions::Add(const Record& session)
{
    const int id = Data::Models::Sessions::Insert(m_options.db, session);

    BOOST_LOG_SEV(logger::get(), Log::info)
        << Log::Property("session_id", id)
        << Log::Property("session_name", session.name)
        << "Added session - loading";

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

    Transaction tx(m_options.db);

    const auto preset_ids = Data::Models::Presets::ClearSessions(m_options.db, id);
    Data::Models::Sessions::Remove(m_options.db, id);

    if (!tx.Commit())
    {
        throw std::runtime_error("Failed to commit session removal transaction");
    }

    BOOST_LOG_SEV(logger::get(), Log::info)
        << Log::Property("session_id", session->id)
        << Log::Property("session_name", session->name)
        << "Removed session";

    for (const auto preset_id : preset_ids)
    {
        Event preset_updated("preset.updated", {{ "preset_id", preset_id }});
        m_options.events.Publish(std::move(preset_updated));
    }

    PublishDetached(id, SessionEvent("session.removed", {{ "session_name", session->name }}));
}

void Sessions::Update(const Record& session)
{
    const auto current     = Find(session.id);
    const bool new_default = session.is_default && current && !current->is_default;

    // moving the default and updating the row must fail together
    Transaction tx(m_options.db);

    if (new_default)
    {
        Data::Models::Sessions::SetDefault(m_options.db, session.id);
    }

    Data::Models::Sessions::Update(m_options.db, session);

    if (!tx.Commit())
    {
        throw std::runtime_error("Failed to commit session update");
    }

    if (new_default)
    {
        BOOST_LOG_SEV(logger::get(), Log::info)
            << Log::Property("session_id", session.id)
            << Log::Property("session_name", session.name)
            << "Set as default session";
    }

    if (const auto instance = Get(session.id))
    {
        instance->Rename(session.name);
        instance->SetTimers(session);

        SessionEvent updated("session.updated");
        updated.session_id = instance->Id();
        updated.session    = instance;

        m_options.events.Publish(std::move(updated));
    }
}


void Sessions::Load(const std::function<void()>& load_callback)
{
    const auto sessions = Data::Models::Sessions::List(m_options.db);

    PORLA_LOG(logger::get(), info, "Loading {count} session(s)", sessions.size());

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
        BOOST_LOG_SEV(logger::get(), Log::warning)
            << Log::Property("session_id", id)
            << "No session found with ID";

        if (load_callback) { boost::asio::post(m_options.io, load_callback); }

        return;
    }

    if (m_sessions.contains(record->id))
    {
        BOOST_LOG_SEV(logger::get(), Log::info)
            << Log::Property("session_id", record->id)
            << Log::Property("session_name", record->name)
            << "Already loaded - skipping";

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
