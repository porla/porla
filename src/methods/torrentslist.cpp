#include "torrentslist.hpp"

#include "../session.hpp"

using porla::Methods::TorrentsList;
using porla::Methods::TorrentsListRes;

TorrentsList::TorrentsList(std::string const& path, ISession& session)
    : Method(boost::beast::http::verb::get, path)
    , m_session(session)
{
}

void TorrentsList::Invoke(WriteCb<TorrentsListRes> cb)
{
    auto const& torrents = m_session.Torrents();

    cb({});
}
