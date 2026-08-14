#include "torrentsadd.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/magnet_uri.hpp>

#include "../../../data/models/presets.hpp"
#include "../../../data/models/sessions.hpp"
#include "../../../sessions.hpp"
#include "../../../torrentclientdata.hpp"
#include "../../../utils/base64.hpp"

namespace lt = libtorrent;

using json = nlohmann::json;

using porla::Rpc::Methods::Torrents::TorrentsAdd;
using porla::Rpc::Methods::Torrents::TorrentsAddReq;

static void ApplyPreset(lt::add_torrent_params& p, const porla::Data::Models::Presets::Preset& preset)
{
    if (preset.download_limit.has_value())  p.download_limit  = preset.download_limit.value();
    if (preset.max_connections.has_value()) p.max_connections = preset.max_connections.value();
    if (preset.max_uploads.has_value())     p.max_uploads     = preset.max_uploads.value();
    if (preset.save_path.has_value())       p.save_path       = preset.save_path.value();
    if (preset.storage_mode.has_value())    p.storage_mode    = lt::storage_mode_sparse; // preset.storage_mode.value();
    if (preset.upload_limit.has_value())    p.upload_limit    = preset.upload_limit.value();

    // Set our custom client data
    if (preset.category.has_value())
        p.userdata.get<porla::TorrentClientData>()->category = preset.category.value();

    if (!preset.tags.empty())
        p.userdata.get<porla::TorrentClientData>()->tags = preset.tags;
}

TorrentsAdd::TorrentsAdd(sqlite3* db, porla::Sessions& sessions)
    : m_db(db)
    , m_sessions(sessions)
{
}

void TorrentsAdd::Execute(const TorrentsAddReq& req, ResponseWriterHandle cb)
{
    if (!req.magnet_uri.has_value() && !req.ti.has_value())
    {
        return cb->Error(-3, "Either 'ti' or 'magnet_uri' must be set");
    }

    // Which session should we add this torrent to?
    // - If we have a session_id, use that
    // - If we have a preset_id, and that preset has a session_id, use that
    // - If there is a default preset, and that preset has a session_id, use that
    // - If nothing, use the default

    const auto& default_preset = Data::Models::Presets::GetDefault(m_db);

    const auto& preset = req.preset_id.has_value()
        ? Data::Models::Presets::GetById(m_db, req.preset_id.value())
        : default_preset;

    const auto session = req.session_id.has_value()
        ? Data::Models::Sessions::GetById(m_db, req.session_id.value())
        : preset.has_value() && preset->session_id.has_value()
            ? Data::Models::Sessions::GetById(m_db, preset->session_id.value())
            : default_preset.has_value() && default_preset->session_id.has_value()
                ? Data::Models::Sessions::GetById(m_db, default_preset->session_id.value())
                : Data::Models::Sessions::GetDefault(m_db);

    if (!session)
    {
        const json error_details = {
            "session_id", req.session_id.has_value()
                ? json(req.session_id.value())
                : preset.has_value() && preset->session_id.has_value()
                    ? json(preset->session_id.value())
                    : default_preset.has_value() && default_preset->session_id.has_value()
                        ? json(default_preset->session_id.value())
                        : json(-1)
        };

        return cb->Error(-1, "Session not found", error_details);
    }

    const auto& session_state = m_sessions.Get(session->id);

    if (session_state == nullptr)
    {
        const json error_details = {
            "session_id", session->id
        };

        return cb->Error(-2, "Session not loaded", error_details);
    }

    lt::add_torrent_params p;
    p.userdata = lt::client_data_t(new TorrentClientData());
    p.userdata.get<TorrentClientData>()->state = session_state;

    if (default_preset.has_value())
    {
        BOOST_LOG_TRIVIAL(info) << "Applying default preset";
        ApplyPreset(p, default_preset.value());
    }

    // Apply the user-specified preset unless it is also the default preset, which has
    // already been applied above.
    if (preset.has_value() && (!default_preset.has_value() || preset->id != default_preset->id))
    {
        BOOST_LOG_TRIVIAL(info) << "Applying preset " << preset->name;
        ApplyPreset(p, preset.value());
    }

    if (req.ti.has_value())
    {
        auto buffer = porla::Utils::Base64::Decode(req.ti.value());

        lt::error_code ec;
        lt::bdecode_node node = lt::bdecode(buffer, ec);

        if (ec)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to decode torrent file: " << ec.message();
            return cb->Error(-10, "Failed to bdecode 'ti' parameter");
        }

        p.ti = std::make_shared<lt::torrent_info>(node, ec);

        if (ec)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to parse torrent file to torrent info: " << ec.message();
            return cb->Error(-11, "Failed to parse torrent_info from bdecoded data");
        }

        if (session_state->torrents.find(p.ti->info_hashes()) != session_state->torrents.end())
        {
            return cb->Error(-6, "Torrent already in session");
        }
    }
    else if (req.magnet_uri.has_value())
    {
        lt::error_code ec;
        lt::parse_magnet_uri(req.magnet_uri.value(), p, ec);

        if (ec)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to parse magnet uri: " << ec.message();
            return cb->Error(-20, "Could not parse 'magnet_uri' param");
        }

        if (session_state->torrents.find(p.info_hashes) != session_state->torrents.end())
        {
            return cb->Error(-6, "Torrent already in session");
        }
    }

    if (req.download_limit.has_value())  p.download_limit  = req.download_limit.value();
    if (req.flags.has_value())           p.flags           = req.flags.value();
    if (req.http_seeds.has_value())      p.http_seeds      = req.http_seeds.value();
    if (req.max_connections.has_value()) p.max_connections = req.max_connections.value();
    if (req.max_uploads.has_value())     p.max_uploads     = req.max_uploads.value();
    if (req.name.has_value())            p.name            = req.name.value();
    if (req.save_path.has_value()
        && req.save_path->length() > 0)  p.save_path       = req.save_path.value();
    if (req.trackers.has_value())        p.trackers        = req.trackers.value();
    if (req.upload_limit.has_value())    p.upload_limit    = req.upload_limit.value();
    if (req.url_seeds.has_value())       p.url_seeds       = req.url_seeds.value();

    // userdata values
    if (req.category.has_value())        p.userdata.get<TorrentClientData>()->category = req.category.value();
    if (req.metadata.has_value())        p.userdata.get<TorrentClientData>()->metadata = req.metadata.value();
    if (req.tags.has_value())            p.userdata.get<TorrentClientData>()->tags     = req.tags.value();

    // Before passing our params to the session. Validate that we have at least
    // an info hash, or
    // a torrent info object, and
    // a save path

    if (p.save_path.empty())
    {
        return cb->Error(-2, "'save_path' missing");
    }

    if (!p.ti && p.info_hashes == lt::info_hash_t())
    {
        return cb->Error(-3, "Either 'ti' or 'magnet_uri' must be set");
    }

    lt::info_hash_t hash;

    try
    {
        session_state->session->async_add_torrent(p);
        hash = p.ti ? p.ti->info_hashes() : p.info_hashes;
    }
    catch (const std::exception& ex)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to add torrent to session: " << ex.what();
        return cb->Error(-4, "Failed to add torrent to session", {"what", ex.what()});
    }

    if (hash == lt::info_hash_t())
    {
        return cb->Error(-5, "Failed to add torrent");
    }

    cb->Ok(TorrentsAddRes{
        .info_hash  = hash,
        .session_id = session_state->id
    });
}
