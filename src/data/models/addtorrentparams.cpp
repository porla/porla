#include "addtorrentparams.hpp"

using porla::Data::Models::AddTorrentParams;

int AddTorrentParams::Count(sqlite3 *db)
{
    return 0;
}

void AddTorrentParams::ForEach(sqlite3 *db,const std::function<void(const libtorrent::add_torrent_params&)>& cb)
{

}
