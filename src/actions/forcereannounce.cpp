#include "forcereannounce.hpp"

#include <boost/asio/deadline_timer.hpp>
#include <boost/log/trivial.hpp>

#include "actioncallback.hpp"
#include "../session.hpp"

using porla::Actions::ForceReannounce;

ForceReannounce::ForceReannounce(porla::ISession& session)
    : m_session(session)
{
}

void ForceReannounce::Invoke(const libtorrent::info_hash_t& hash, const toml::array& args, const std::shared_ptr<ActionCallback>& callback)
{
    auto const& torrents = m_session.Torrents();
    auto const& torrent = torrents.find(hash);

    if (torrent == torrents.end())
    {
        return;
    }

    BOOST_LOG_TRIVIAL(debug) << "Forcing reannounce for " << torrent->second.status().name;

    torrent->second.force_reannounce();
}
