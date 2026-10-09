#include "torrentspropertiesget.hpp"

#include "../../../data/models/sessions.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrent.hpp"

using porla::Rpc::Methods::Torrents::TorrentsPropertiesGet;
using porla::Rpc::Methods::Torrents::TorrentsPropertiesGetReq;
using porla::Rpc::Methods::Torrents::TorrentsPropertiesGetRes;

TorrentsPropertiesGet::TorrentsPropertiesGet(sqlite3* db, porla::Sessions& sessions)
    : m_db(db)
    , m_sessions(sessions)
{
}

void TorrentsPropertiesGet::Execute(const TorrentsPropertiesGetReq& req, ResponseWriterHandle cb)
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

    if (!it->second.status.handle.is_valid())
    {
        return cb->Error(-4, "Torrent not valid");
    }

    cb->Ok(TorrentsPropertiesGetRes{
        .download_limit  = it->second.status.handle.download_limit(),
        .flags           = it->second.status.handle.flags(),
        .max_connections = it->second.status.handle.max_connections(),
        .max_uploads     = it->second.status.handle.max_uploads(),
        .upload_limit    = it->second.status.handle.upload_limit()
    });
}
