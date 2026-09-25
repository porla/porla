#include "torrentspropertiesset.hpp"

#include "../../../data/models/addtorrentparams.hpp"
#include "../../../data/models/sessions.hpp"
#include "../../../sessions.hpp"
#include "../../../torrentclientdata.hpp"

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

    const auto it = session_state->torrents.find(req.info_hash);

    if (it == session_state->torrents.end())
    {
        return cb->Error(-3, "Torrent not found in session");
    }

    if (!it->second.handle.is_valid())
    {
        return cb->Error(-4, "Torrent not valid");
    }

    const auto client_data = it->second.handle.userdata().get<TorrentClientData>();

    if (const auto val = req.download_limit)
        it->second.handle.set_download_limit(*val);

    if (req.flags.has_value() && req.flags_mask.has_value())
    {
        const auto flags = req.flags.value();
        const auto mask  = req.flags_mask.value();

        it->second.handle.set_flags(flags, mask);
    }

    if (const auto val = req.max_connections)
        it->second.handle.set_max_connections(*val);

    if (const auto val = req.max_uploads)
        it->second.handle.set_max_uploads(*val);

    if (const auto val = req.upload_limit)
        it->second.handle.set_upload_limit(*val);

    if (req.category.has_value() || req.tags.has_value())
    {
        if (client_data == nullptr)
        {
            return cb->Error(-5, "Torrent has no client data - cannot set category or tags");
        }

        if (req.category.has_value())
        {
            client_data->category = req.category.value();
        }

        if (req.tags.has_value())
        {
            client_data->tags = req.tags.value();
        }

        Data::Models::AddTorrentParams::UpdateClientData(
            m_db,
            session_state->id,
            it->second.info_hashes,
            *client_data);
    }

    cb->Ok({});
}
