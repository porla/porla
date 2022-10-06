#include "torrentsadd.hpp"

#include <libtorrent/add_torrent_params.hpp>

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

void TorrentsAdd::Invoke(TorrentsAddReq const& req)
{
    lt::error_code ec;
    lt::bdecode_node node = lt::bdecode(
        porla::Utils::Base64::Decode(req.ti),
        ec);

    lt::add_torrent_params p;
    p.save_path = req.save_path;
    p.ti = std::make_shared<lt::torrent_info>(node);

    m_session.AddTorrent(p);
}
