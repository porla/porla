#include "torrentsmigrate.hpp"

#include <boost/log/trivial.hpp>

#include "../../../data/models/sessions.hpp"
#include "../../../sessions.hpp"
#include "../../../torrentclientdata.hpp"

using porla::Rpc::Methods::Torrents::TorrentsMigrate;
using porla::Rpc::Methods::Torrents::TorrentsMigrateReq;
using porla::Rpc::Methods::Torrents::TorrentsMigrateRes;

struct RemoveState
{
    std::shared_ptr<boost::signals2::connection> connection;
    lt::add_torrent_params                       params;
    std::weak_ptr<TorrentsMigrate>               self;
    int                                          source_session_id;
    int                                          target_session_id;
    porla::Rpc::ResponseWriterHandle             writer;
};

TorrentsMigrate::TorrentsMigrate(sqlite3* db, porla::Sessions &sessions)
    : m_db(db)
    , m_sessions(sessions)
{
}

void TorrentsMigrate::Execute(const TorrentsMigrateReq &req, ResponseWriterHandle cb)
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

    auto [ th, _ ] = handle->second;

    lt::add_torrent_params params = th.get_resume_data();
    params.userdata = lt::client_data_t(new TorrentClientData());
    params.userdata.get<TorrentClientData>()->category = th.userdata().get<TorrentClientData>()->category;
    params.userdata.get<TorrentClientData>()->metadata = th.userdata().get<TorrentClientData>()->metadata;
    params.userdata.get<TorrentClientData>()->tags     = th.userdata().get<TorrentClientData>()->tags;

    RemoveState state{
        .connection        = std::make_shared<boost::signals2::connection>(),
        .params            = params,
        .self              = weak_from_this(),
        .source_session_id = session_state->id,
        .target_session_id = req.target_session_id,
        .writer            = cb
    };

    *state.connection = m_sessions.OnTorrentRemoved([state](auto session_state, auto removed_hash)
    {
        auto self = state.self.lock();

        if (self == nullptr)
        {
            state.connection->disconnect();
            state.writer->Error(-1, "Failed to lock self");
            return;
        }

        if (session_state->id == state.source_session_id && removed_hash == state.params.info_hashes)
        {
            state.connection->disconnect();

            auto target_session = self->m_sessions.Get(state.target_session_id);

            if (target_session == nullptr)
            {
                BOOST_LOG_TRIVIAL(warning) << "Target session not found - torrent must be added manually";
                state.writer->Error(-2, "Target session not found - torrent must be added manually");
                return;
            }

            state.params.userdata.get<TorrentClientData>()->state = target_session;

            target_session->session->async_add_torrent(state.params);

            BOOST_LOG_TRIVIAL(info) << "Torrent migrated to session " << target_session->name;

            state.writer->Ok({});
        }
    });

    session_state->session->remove_torrent(th);
}
