#include "torrentsmigrate.hpp"

#include <boost/log/trivial.hpp>
#include <boost/signals2.hpp>

#include "../../../data/models/sessions.hpp"
#include "../../../events.hpp"
#include "../../../sessions/session.hpp"
#include "../../../sessions/sessions.hpp"
#include "../../../sessions/torrentevent.hpp"
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

TorrentsMigrate::TorrentsMigrate(sqlite3* db, porla::Events& events, porla::Sessions &sessions)
    : m_db(db)
    , m_events(events)
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

    if (!m_sessions.Get(req.target_session_id))
    {
        return cb->Error(-2, "Target session not loaded");
    }

    const auto torrent = session_state->Find(req.info_hash);

    if (torrent == nullptr)
    {
        return cb->Error(-3, "Torrent not found in session");
    }

    const auto prev_client_data = torrent->status.handle.userdata().get<TorrentClientData>();
          auto client_data      = new TorrentClientData();

    if (prev_client_data)
    {
        client_data->category     = prev_client_data->category;
        client_data->completed_at = prev_client_data->completed_at;
        client_data->metadata     = prev_client_data->metadata;
        client_data->tags         = prev_client_data->tags;
    }

    lt::add_torrent_params params = torrent->status.handle.get_resume_data();
    params.userdata = lt::client_data_t(client_data);

    RemoveState state{
        .connection        = std::make_shared<boost::signals2::connection>(),
        .params            = params,
        .self              = weak_from_this(),
        .source_session_id = session_state->Id(),
        .target_session_id = req.target_session_id,
        .writer            = cb
    };

    *state.connection = m_events.On("torrent.removed", [state](const porla::Event& event)
    {
        const auto& removed = static_cast<const porla::TorrentEvent&>(event);

        auto self = state.self.lock();

        if (self == nullptr)
        {
            state.connection->disconnect();
            state.writer->Error(-1, "Failed to lock self");
            return;
        }

        if (removed.session_id == state.source_session_id && removed.info_hash == state.params.info_hashes)
        {
            state.connection->disconnect();

            auto target_session = self->m_sessions.Get(state.target_session_id);

            if (target_session == nullptr)
            {
                BOOST_LOG_TRIVIAL(warning) << "Target session not found - torrent must be added manually";
                state.writer->Error(-2, "Target session not found - torrent must be added manually");
                return;
            }

            state.params.userdata.get<TorrentClientData>()->session = target_session;

            target_session->Libtorrent().async_add_torrent(state.params);

            BOOST_LOG_TRIVIAL(info) << "Torrent migrated to session " << target_session->Name();

            state.writer->Ok({});
        }
    });

    session_state->Libtorrent().remove_torrent(torrent->status.handle);
}
