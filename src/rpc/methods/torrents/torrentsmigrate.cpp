#include "torrentsmigrate.hpp"

#include <boost/asio/steady_timer.hpp>
#include <boost/log/trivial.hpp>
#include <boost/signals2.hpp>

#include "resolve.hpp"

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
    porla::TorrentClientData                     client_data = {};
    std::shared_ptr<boost::signals2::connection> connection;
    bool                                         did_add  = false;
    bool                                         did_fail = false;
    lt::add_torrent_params                       params;
    std::weak_ptr<porla::Session>                source_session;
    int                                          source_session_id;
    std::weak_ptr<porla::Session>                target_session;
    std::unique_ptr<boost::asio::steady_timer>   timeout;
    porla::Rpc::ResponseWriterHandle             writer;

    void Fail()
    {
        if (did_add || did_fail) { return; }
        did_fail = true;

        timeout->cancel();

        auto session = source_session.lock();

        // target session did not exist, or it did not accept the torrent
        if (session)
        {
            params.userdata = lt::client_data_t(new porla::TorrentClientData(client_data));

            if (session->Add(params))
            {
                BOOST_LOG_TRIVIAL(warning) << "Target session did not exist or did not accept torrent - added back to source";

                return writer->Error(-6, "Torrent migration failed - added back to source session");
            }
        }

        // We now have a failure on our hand, a torrent is missing.
        BOOST_LOG_TRIVIAL(error)
            << "Failed to migrate torrent and source did not accept it back - lost torrent " << params.info_hashes << " at " << params.save_path;

        return writer->Error(-7, "Migration failed - neither target nor source accepted the torrent");
    }

    void TimedOut()
    {
        if (did_add || did_fail) { return; }
        did_fail = true;
        connection->disconnect();

        BOOST_LOG_TRIVIAL(error) << "Timed out waiting for removal of " << params.info_hashes << " at " << params.save_path;

        writer->Error(-32603, "Timed out waiting for removal");
    }
};

TorrentsMigrate::TorrentsMigrate(boost::asio::io_context& io, porla::Events& events, porla::Sessions &sessions)
    : m_io(io)
    , m_events(events)
    , m_sessions(sessions)
{
}

void TorrentsMigrate::Execute(const TorrentsMigrateReq &req, ResponseWriterHandle cb)
{
    const auto resolved = ResolveTorrent(m_sessions, req.session_id, req.info_hash, cb);

    if (!resolved)
    {
        return;
    }

    if (req.target_session_id == resolved->session->Id())
    {
        return cb->Error(-32602, "target_session_id must differ from session_id", {{"field", "target_session_id"}});
    }

    auto target_session = m_sessions.Get(req.target_session_id);

    if (!target_session)
    {
        return cb->Error(-2, "Target session not loaded");
    }

    if (target_session->Contains(resolved->torrent->status.info_hashes))
    {
        return cb->Error(-5, "Torrent already in target session");
    }

    auto state = std::make_shared<RemoveState>(RemoveState{
        .connection        = std::make_shared<boost::signals2::connection>(),
        .params            = resolved->torrent->status.handle.get_resume_data(),
        .source_session    = resolved->session,
        .source_session_id = resolved->session->Id(),
        .target_session    = target_session,
        .timeout           = std::make_unique<boost::asio::steady_timer>(m_io),
        .writer            = cb
    });

    const auto prev_client_data = resolved->torrent->status.handle.userdata().get<TorrentClientData>();

    if (prev_client_data)
    {
        state->client_data.category     = prev_client_data->category;
        state->client_data.completed_at = prev_client_data->completed_at;
        state->client_data.metadata     = prev_client_data->metadata;
        state->client_data.tags         = prev_client_data->tags;
    }

    *state->connection = m_events.On("torrent.removed",
        [state](const porla::Event& event)
        {
            if (state->did_fail) return;

            const auto& removed = static_cast<const porla::TorrentEvent&>(event);

            if (removed.session_id == state->source_session_id && removed.info_hash == state->params.info_hashes)
            {
                auto source_session = state->source_session.lock();
                auto target_session = state->target_session.lock();

                state->connection->disconnect();

                // Best case, target session exists and accepts the torrent
                if (target_session)
                {
                    state->params.userdata = lt::client_data_t(new TorrentClientData(state->client_data));

                    if (target_session->Add(state->params))
                    {
                        state->did_add = true;
                        state->timeout->cancel();

                        BOOST_LOG_TRIVIAL(info) << "Torrent " << state->params.info_hashes << " migrated to session " << target_session->Name();

                        return state->writer->Ok(TorrentsMigrateRes{
                            .info_hash  = state->params.info_hashes,
                            .session_id = target_session->Id()
                        });
                    }
                }

                state->Fail();
            }
        });

    resolved->session->Libtorrent().remove_torrent(resolved->torrent->status.handle);

    state->timeout->expires_after(std::chrono::seconds(30));
    state->timeout->async_wait([state](const boost::system::error_code& ec)
    {
        if (ec) { return; }
        state->TimedOut();
    });
}
