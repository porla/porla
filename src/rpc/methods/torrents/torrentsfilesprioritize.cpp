#include "torrentsfilesprioritize.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions.hpp"

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

    const auto& handle = session_state->torrents.find(req.info_hash);

    if (handle == session_state->torrents.end())
    {
        return cb->Error(-3, "Torrent not found in session");
    }

    const auto& [ th, _ ] = handle->second;

    std::vector<lt::download_priority_t> file_prios = th.get_file_priorities();

    for (const auto& fp : req.priorities)
    {
        const auto index = static_cast<int>(fp.index);

        if (index < 0 || index >= file_prios.size())
        {
            continue;
        }

        file_prios[index] = fp.priority;
    }

    th.prioritize_files(file_prios);

    cb->Ok({});
}
