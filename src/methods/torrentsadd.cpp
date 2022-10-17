#include "torrentsadd.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/magnet_uri.hpp>

#include "../session.hpp"
#include "../utils/base64.hpp"

namespace lt = libtorrent;

using porla::Methods::TorrentsAdd;
using porla::Methods::TorrentsAddReq;

TorrentsAdd::TorrentsAdd(ISession& session, toml::table& cfg)
    : m_session(session)
    , m_cfg(cfg)
{
}

void TorrentsAdd::Invoke(const TorrentsAddReq& req, WriteCb<TorrentsAddRes> cb)
{
    lt::add_torrent_params p;

    if (req.preset.has_value())
    {
        std::string presetName = req.preset.value();

        if (toml::table* maybePreset = m_cfg["preset"][presetName].as_table())
        {
            toml::table& preset = *maybePreset;

            if (auto downLimit = preset["dl_limit"].value<int>())
                p.download_limit = *downLimit;

            if (auto maxConnections = preset["max_connections"].value<int>())
                p.max_connections = *maxConnections;

            if (auto maxUploads = preset["max_uploads"].value<int>())
                p.max_uploads = *maxUploads;

            if (auto savePath = preset["save_path"].value<std::string>())
                p.save_path = *savePath;

            if (auto upLimit = preset["ul_limit"].value<int>())
                p.upload_limit = *upLimit;

            if (const toml::array* trackers = preset["trackers"].as_array())
            {
                for (auto&& item : *trackers)
                {
                    if (!item.is_string()) continue;
                    p.trackers.push_back(*item.value<std::string>());
                }
            }
        }
        else
        {
            BOOST_LOG_TRIVIAL(warning) << "Specified preset '" << presetName << "' is invalid.";
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

    lt::info_hash_t hash = m_session.AddTorrent(p);

    if (hash == lt::info_hash_t())
    {
        return cb.Error(-4, "Failed to add torrent");
    }

    cb.Ok(TorrentsAddRes{
        .info_hash = hash
    });
}
