#include "torrentsadd.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/magnet_uri.hpp>

#include "../sessions.hpp"
#include "../torrentclientdata.hpp"
#include "../utils/base64.hpp"

namespace lt = libtorrent;

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

TorrentsAdd::TorrentsAdd(Sessions& sessions, const std::map<std::string, Config::Preset>& presets)
    : m_sessions(sessions)
    , m_presets(presets)
{
}

void TorrentsAdd::Invoke(const TorrentsAddReq& req, WriteCb<TorrentsAddRes> cb)
{
    const auto& state = req.preset.has_value()
        ? m_presets.find(req.preset.value()) != m_presets.end()
            ? m_presets.at(req.preset.value()).session.has_value()
                ? m_sessions.Get(m_presets.at(req.preset.value()).session.value())
                : m_sessions.Default()
            : m_sessions.Default()
        : m_presets.find("default") != m_presets.end()
            ? m_presets.at("default").session.has_value()
                ? m_sessions.Get(m_presets.at("default").session.value())
                : m_sessions.Default()
            : m_sessions.Default();

    if (state == nullptr)
    {
        return cb.Error(-10, "Session not found");
    }

    lt::add_torrent_params p;
    p.userdata = lt::client_data_t(new TorrentClientData());
    p.userdata.get<TorrentClientData>()->state = state;

    // Apply the 'default' preset if it exists
    if (m_presets.find("default") != m_presets.end())
    {
        BOOST_LOG_TRIVIAL(debug) << "Applying default preset";
        ApplyPreset(p, m_presets.at("default"));
    }

    if (req.preset.has_value() && !req.preset.value().empty())
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

    if (req.ti.has_value())
    {
        auto buffer = porla::Utils::Base64::Decode(req.ti.value());

        lt::error_code ec;
        lt::bdecode_node node = lt::bdecode(buffer, ec);

        if (ec)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to decode torrent file: " << ec.message();
            return cb.Error(-1, "Failed to bdecode 'ti' parameter");
        }

        p.ti = std::make_shared<lt::torrent_info>(node, ec);

        if (ec)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to parse torrent file to info: " << ec.message();
            return cb.Error(-2, "Failed to parse torrent_info from bdecoded data");
        }

        for (const auto& [ name, s ]: m_sessions.All())
        {
            if (s->torrents.find(p.ti->info_hashes()) != s->torrents.end())
            {
                return cb.Error(-3, "Torrent already in session '" + name + "'");
            }
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
        return cb.Error(-4, "'save_path' missing");
    }

    if (!p.ti && p.info_hashes == lt::info_hash_t())
    {
        return cb.Error(-4, "Either 'ti' or 'magnet_uri' must be set");
    }

    lt::info_hash_t hash;

    try
    {
        state->session->async_add_torrent(p);
        hash = p.ti ? p.ti->info_hashes() : p.info_hashes;
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

    cb.Ok(TorrentsAddRes{
        .info_hash = hash
    });
}
