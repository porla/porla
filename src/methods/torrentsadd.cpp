#include "torrentsadd.hpp"

#include <boost/log/trivial.hpp>
#include <libtorrent/add_torrent_params.hpp>
#include <libtorrent/magnet_uri.hpp>

#include "../session.hpp"
#include "../utils/base64.hpp"

namespace lt = libtorrent;

using porla::Methods::TorrentsAdd;
using porla::Methods::TorrentsAddReq;

TorrentsAdd::TorrentsAdd(std::string const& path, ISession& session)
    : MethodT(boost::beast::http::verb::post, path)
    , m_session(session)
{
}

void TorrentsAdd::Invoke(TorrentsAddReq const& req, WriteCb<TorrentsAddRes> cb)
{
    lt::add_torrent_params p;

    if (req.ti.has_value()) {
        auto buffer = porla::Utils::Base64::Decode(req.ti.value());

        lt::error_code ec;
        lt::bdecode_node node = lt::bdecode(buffer, ec);

        if (ec) {
            BOOST_LOG_TRIVIAL(error) << "Failed to decode torrent file: " << ec.message();
            cb({});
            return;
        }

        p.ti = std::make_shared<lt::torrent_info>(node, ec);

        if (ec)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to parse torrent file to info: " << ec.message();
            cb({});
            return;
        }
    }
    else if (req.magnet_uri.has_value())
    {
        lt::error_code ec;
        lt::parse_magnet_uri(req.magnet_uri.value(), p, ec);

        if (ec)
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to parse magnet uri: " << ec.message();
            cb({});
            return;
        }
    }

    p.save_path = req.save_path;

    lt::info_hash_t hash = m_session.AddTorrent(p);

    if (hash == lt::info_hash_t())
    {
        cb({});
        return;
    }

    cb(TorrentsAddRes{
        .info_hash = hash
    });
}
