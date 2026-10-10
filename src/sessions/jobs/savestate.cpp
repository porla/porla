#include "savestate.hpp"

#include <boost/log/trivial.hpp>

#include "../session.hpp"

using porla::Jobs::SaveState;

porla::Job::Result SaveState::Run(Session& session)
{
    session.Persist();
    session.UntrackInvalidTorrents();

    const std::vector<lt::torrent_status> torrents = session.Libtorrent().get_torrent_status(
        [](const lt::torrent_status& ts)
        {
            return bool(ts.need_save_resume_data & lt::torrent_handle::only_if_modified);
        });

    if (torrents.empty())
    {
        return Done();
    }

    BOOST_LOG_TRIVIAL(info) << session.Log()
        << "Saving state for " << torrents.size() << " torrent(s)";

    for (const auto& ts : torrents)
    {
        ts.handle.save_resume_data(lt::torrent_handle::only_if_modified);
    }

    return Done();
}
