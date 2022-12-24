#pragma once

#include <map>
#include <memory>
#include <string>

#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <libtorrent/torrent_status.hpp>
#include <sqlite3.h>

#include "../config.hpp"

namespace porla
{
    class ISession;
}

namespace porla::Actions
{
    class Action;

    struct ExecutorOptions
    {
        sqlite3* db;
        boost::asio::io_context& io;
        std::map<std::string, porla::Config::Preset> presets;
        porla::ISession& session;
        std::map<std::string, std::shared_ptr<Action>> actions;
    };

    class Executor
    {
    public:
        explicit Executor(const ExecutorOptions& options);
        ~Executor();

    private:

        void OnTorrentAdded(const libtorrent::torrent_status& ts);
        void OnTorrentFinished(const libtorrent::torrent_status& ts);

        void Run(
            const libtorrent::torrent_status& ts,
            const std::function<std::vector<Config::PresetAction>(const Config::Preset&)>& selector);

        sqlite3* m_db;
        boost::asio::io_context& m_io;
        ISession& m_session;

        std::map<std::string, std::shared_ptr<Action>> m_actions;
        std::map<std::string, Config::Preset> m_presets;

        boost::signals2::connection m_torrent_added_connection;
        boost::signals2::connection m_torrent_finished_connection;
    };
}