#include "torrentsfilesprioritize.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsFilesPrioritize;
using porla::Rpc::Methods::Torrents::TorrentsFilesPrioritizeReq;
using porla::Rpc::Methods::Torrents::TorrentsFilesPrioritizeRes;

TorrentsFilesPrioritize::TorrentsFilesPrioritize(sqlite3* db, porla::Sessions& sessions)
    : m_db(db)
    , m_sessions(sessions)
{
}

void TorrentsFilesPrioritize::Execute(const TorrentsFilesPrioritizeReq& req, ResponseWriterHandle cb)
{
    const auto session = req.session_id.has_value()
        ? Data::Models::Sessions::GetById(m_db, req.session_id.value())
        : Data::Models::Sessions::GetDefault(m_db);

    if (!session)
    {
        return cb->Error(-1, "Session not found");
    }

    const auto& session_state = m_sessions.Get(session->id);

    if (session_state == nullptr)
    {
        return cb->Error(-2, "Session not loaded");
    }

    const auto it = session_state->Torrents().find(req.info_hash);

    if (it == session_state->Torrents().end())
    {
        return cb->Error(-3, "Torrent not found in session");
    }

    std::vector<lt::download_priority_t> file_prios = it->second.status.handle.get_file_priorities();

    for (const auto& fp : req.priorities)
    {
        const auto index = static_cast<int>(fp.index);

        if (index < 0 || index >= file_prios.size())
        {
            continue;
        }

        file_prios[index] = fp.priority;
    }

    it->second.status.handle.prioritize_files(file_prios);

    cb->Ok({});
}
