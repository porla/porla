#include "torrentspropertiesset.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions.hpp"

using porla::Rpc::Methods::Torrents::TorrentsPropertiesSet;
using porla::Rpc::Methods::Torrents::TorrentsPropertiesSetReq;
using porla::Rpc::Methods::Torrents::TorrentsPropertiesSetRes;

TorrentsPropertiesSet::TorrentsPropertiesSet(sqlite3* db, porla::Sessions& sessions)
    : m_db(db)
    , m_sessions(sessions)
{
}

void TorrentsPropertiesSet::Execute(const TorrentsPropertiesSetReq& req, ResponseWriterHandle cb)
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

    if (!th.is_valid())
    {
        return cb->Error(-4, "Torrent not valid");
    }

    if (const auto val = req.download_limit)
        th.set_download_limit(*val);

    if (const auto val = req.flags)
    {
        lt::torrent_flags_t mask  = {};
        th.set_flags(*val, mask);
    }

    if (const auto val = req.max_connections)
        th.set_max_connections(*val);

    if (const auto val = req.max_uploads)
        th.set_max_uploads(*val);

    if (const auto val = req.upload_limit)
        th.set_upload_limit(*val);

    cb->Ok({});
}
