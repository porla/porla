#pragma once

#include <map>
#include <memory>
#include <vector>

#include <boost/asio.hpp>
#include <libtorrent/session.hpp>
#include <sqlite3.h>

namespace porla
{
    struct SessionOptions
    {
        sqlite3* db;
    };

    class ISession
    {
    public:
        virtual void AddTorrent(libtorrent::add_torrent_params const& p) = 0;
        virtual void Query(const std::string_view& query, const std::function<int(sqlite3_stmt*)>& cb) = 0;
        virtual const std::map<lt::info_hash_t, lt::torrent_status>& Torrents() = 0;
    };

    class Session : public ISession
    {
    public:
        explicit Session(boost::asio::io_context& io, SessionOptions const& options);
        ~Session();

        void Load();

        void AddTorrent(libtorrent::add_torrent_params const& p) override;
        void Query(const std::string_view& query, const std::function<int(sqlite3_stmt*)>& cb) override;
        const std::map<lt::info_hash_t, lt::torrent_status>& Torrents() override;

    private:
        void ReadAlerts();
        void PostUpdates(boost::system::error_code ec);

        boost::asio::io_context& m_io;
        boost::asio::deadline_timer m_timer;

        sqlite3* m_db;
        sqlite3* m_tdb;

        std::unique_ptr<libtorrent::session> m_session;
        std::map<libtorrent::info_hash_t, libtorrent::torrent_status> m_torrents;
    };
}
