#include "torrentsadd.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/magnet_uri.hpp>

#include "../data/models/torrentsmetadata.hpp"
#include "../session.hpp"
#include "../torrentclientdata.hpp"
#include "../utils/base64.hpp"

namespace lt = libtorrent;

using porla::Data::Models::TorrentsMetadata;
using porla::Methods::TorrentsAdd;
using porla::Methods::TorrentsAddReq;

static void ApplyPreset(lt::add_torrent_params& p, const porla::Config::Preset& preset)
{
    if (preset.download_limit.has_value())  p.download_limit  = preset.download_limit.value();
    if (preset.max_connections.has_value()) p.max_connections = preset.max_connections.value();
    if (preset.max_uploads.has_value())     p.max_uploads     = preset.max_uploads.value();
    if (preset.save_path.has_value())       p.save_path       = preset.save_path.value();
    if (preset.storage_mode.has_value())    p.storage_mode    = preset.storage_mode.value();
    if (preset.upload_limit.has_value())    p.upload_limit    = preset.upload_limit.value();

    // Set our custom client data
    if (preset.category.has_value())
        p.userdata.get<porla::TorrentClientData>()->category = preset.category.value();

    if (!preset.tags.empty())
        p.userdata.get<porla::TorrentClientData>()->tags = preset.tags;
}

TorrentsAdd::TorrentsAdd(sqlite3* db, ISession& session, const std::map<std::string, Config::Preset>& presets)
    : m_db(db)
    , m_session(session)
    , m_presets(presets)
{
}

void TorrentsAdd::Invoke(const TorrentsAddReq& req, WriteCb<TorrentsAddRes> cb)
{
    lt::add_torrent_params p;
    p.userdata = lt::client_data_t(new TorrentClientData());

    // Apply the 'default' preset if it exists
    if (m_presets.find("default") != m_presets.end())
    {
        ApplyPreset(p, m_presets.at("default"));
    }

    if (req.preset.has_value())
    {
        auto const preset_name = req.preset.value();
        auto const preset = m_presets.find(preset_name);

        if (preset == m_presets.end())
        {
            BOOST_LOG_TRIVIAL(warning) << "Specified preset '" << preset_name << "' not found.";
        }
        // Only apply presets other than default here, since default is applied above..
        else if (preset_name != "default")
        {
            BOOST_LOG_TRIVIAL(debug) << "Applying preset " << preset_name;
            ApplyPreset(p, preset->second);
        }
    }

    if (req.ti.has_value()) {
        auto buffer = porla::Utils::Base64::Decode(req.ti.value());

        lt::error_code ec;
        lt::bdecode_node node = lt::bdecode(buffer, ec);

        if (ec) {
            BOOST_LOG_TRIVIAL(error) << "Failed to decode torrent file: " << ec.message();
            return cb.Error(-1, "Failed to bdecode 'ti' parameter");
        }

        p.ti = std::make_shared<lt::torrent_info>(node, ec);

        if (ec)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to parse torrent file to info: " << ec.message();
            return cb.Error(-2, "Failed to parse torrent_info from bdecoded data");
        }
    }
    else if (req.magnet_uri.has_value())
    {
        lt::error_code ec;
        lt::parse_magnet_uri(req.magnet_uri.value(), p, ec);

        if (ec)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to parse magnet uri: " << ec.message();
            return cb.Error(-3, "Could not parse 'magnet_uri' param");
        }
    }

    if (req.download_limit.has_value())  p.download_limit  = req.download_limit.value();
    if (req.http_seeds.has_value())      p.http_seeds      = req.http_seeds.value();
    if (req.max_connections.has_value()) p.max_connections = req.max_connections.value();
    if (req.max_uploads.has_value())     p.max_uploads     = req.max_uploads.value();
    if (req.name.has_value())            p.name            = req.name.value();
    if (req.save_path.has_value())       p.save_path       = req.save_path.value();
    if (req.trackers.has_value())        p.trackers        = req.trackers.value();
    if (req.upload_limit.has_value())    p.upload_limit    = req.upload_limit.value();
    if (req.url_seeds.has_value())       p.url_seeds       = req.url_seeds.value();

    // Before passing our params to the session. Validate that we have at least
    // an info hash, or
    // a torrent info object, and
    // a save path

    if (p.save_path.empty())
    {
        return cb.Error(-4, "'save_path' missing");
    }

    if (!p.ti && p.info_hashes == lt::info_hash_t())
    {
        return cb.Error(-4, "Either 'ti' or 'magnet_uri' must be set");
    }

    lt::info_hash_t hash;

    try
    {
        hash = m_session.AddTorrent(p);
    }
    catch (const std::exception& ex)
    {
        BOOST_LOG_TRIVIAL(error) << "Failed to add torrent to session: " << ex.what();
        return cb.Error(-5, "Failed to add torrent to session");
    }

    if (hash == lt::info_hash_t())
    {
        return cb.Error(-4, "Failed to add torrent");
    }

    // Set metadata if we successfully added the torrent
    if (auto metadata = req.metadata)
    {
        for (auto const& [key, value] : metadata.value())
        {
            TorrentsMetadata::Set(m_db, hash, key, value);
        }
    }

    // Set static metadata, preset etc
    if (auto val = req.preset)
        TorrentsMetadata::Set(m_db, hash, "preset", json(req.preset.value()));

    cb.Ok(TorrentsAddRes{
        .info_hash = hash
    });
}
