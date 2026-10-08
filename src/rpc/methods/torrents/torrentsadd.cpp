#include "torrentsadd.hpp"

#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/load_torrent.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <sodium.h>

#include "../../../torrentclientdata.hpp"
#include "../../../torrents.hpp"

namespace lt = libtorrent;

using json = nlohmann::json;

using porla::Rpc::Methods::Torrents::TorrentsAdd;
using porla::Rpc::Methods::Torrents::TorrentsAddReq;

namespace
{
    void WriteError(const porla::TorrentsAddResult& result, porla::Rpc::ResponseWriterHandle& cb)
    {
        using Error = porla::TorrentsAddResult::Error;

        switch(result.error)
        {
        case Error::SessionNotFound:  return cb->Error(-1, result.what, {{ "session_id", result.session_id }});
        case Error::SessionNotLoaded: return cb->Error(-2, result.what, {{ "session_id", result.session_id }});
        case Error::MissingInfoHash:  return cb->Error(-4, result.what);
        case Error::AlreadyInSession: return cb->Error(-5, result.what);
        case Error::MissingSavePath:  return cb->Error(-6, result.what);
        case Error::Failed:           return cb->Error(-6, "Failed to add torrent to session", {{ "what", result.what }});
        case Error::None:             return;
        }
    }
}

TorrentsAdd::TorrentsAdd(porla::Torrents& torrents)
    : m_torrents(torrents)
{
}

void TorrentsAdd::Execute(const TorrentsAddReq& req, ResponseWriterHandle cb)
{
    lt::error_code ec;
    lt::add_torrent_params p;
    
    if (req.ti.has_value())
    {
        const auto data = req.ti.value();

        std::string output;
        output.resize(data.size() / 4 * 3 + 3);

        std::size_t bin_len = 0;

        const int result = sodium_base642bin(
            reinterpret_cast<unsigned char*>(output.data()),
            output.size(),
            data.data(),
            data.size(),
            nullptr,
            &bin_len,
            nullptr,
            sodium_base64_VARIANT_ORIGINAL);

        if (result != 0)
        {
            return cb->Error(-2, "Failed to parse 'ti'");
        }

        output.resize(bin_len);

        p = lt::load_torrent_buffer(output, ec, {});

        if (ec)
        {
            return cb->Error(-2, "Failed to parse 'ti'");
        }
    }
    else if (req.magnet_uri.has_value())
    {
        p = lt::parse_magnet_uri(req.magnet_uri.value(), ec);

        if (ec)
        {
            return cb->Error(-2, "Failed to parse 'magnet_uri'");
        }
    }
    else
    {
        return cb->Error(-3, "Either 'ti' or 'magnet_uri' must be set");
    }

    const auto prepared = m_torrents.Prepare(p, porla::TorrentsAddOptions{
        .session_id = req.session_id,
        .preset_id  = req.preset_id,
        .preset     = req.preset
    });

    if (!prepared)
    {
        return WriteError(prepared, cb);
    }

    // explicit values
    if (req.download_limit.has_value())  p.download_limit  = req.download_limit.value();
    if (req.flags.has_value())           p.flags           = req.flags.value();
    if (req.max_connections.has_value()) p.max_connections = req.max_connections.value();
    if (req.max_uploads.has_value())     p.max_uploads     = req.max_uploads.value();
    if (req.name.has_value())            p.name            = req.name.value();
    if (req.save_path.has_value()
        && req.save_path->length() > 0)  p.save_path       = req.save_path.value();
    if (req.trackers.has_value())        p.trackers        = req.trackers.value();
    if (req.upload_limit.has_value())    p.upload_limit    = req.upload_limit.value();
    if (req.url_seeds.has_value())       p.url_seeds       = req.url_seeds.value();

    // userdata values
    auto* client_data = p.userdata.get<TorrentClientData>();

    if (req.category.has_value()) client_data->category = req.category.value();
    if (req.metadata.has_value()) client_data->metadata = req.metadata.value();
    if (req.tags.has_value())     client_data->tags     = req.tags.value();

    const auto added = m_torrents.Add(std::move(p));

    if (!added)
    {
        return WriteError(added, cb);
    }

    cb->Ok(TorrentsAddRes{
        .info_hash  = added.info_hash,
        .session_id = added.session_id
    });
}
