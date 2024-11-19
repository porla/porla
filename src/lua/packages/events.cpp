#include "../packages.hpp"

#include <boost/log/trivial.hpp>
#include <boost/signals2.hpp>
#include <libtorrent/alert_types.hpp>

#include "../plugin.hpp"
#include "../../sessions.hpp"

using porla::Lua::Packages::Events;

struct SignalConnection
{
    explicit SignalConnection(boost::signals2::connection c)
        : connection(std::move(c))
    {
    }

    ~SignalConnection()
    {
        BOOST_LOG_TRIVIAL(trace) << "Disconnecting signal";
        if (connection.connected()) connection.disconnect();
    }

    boost::signals2::connection connection;
};

void Events::Register(sol::state& lua)
{
    auto type = lua.new_usertype<SignalConnection>(
        "events.SignalConnection",
        sol::no_constructor);

    type["disconnect"] = [](const std::shared_ptr<SignalConnection>& c)
    {
        c->connection.disconnect();
    };

    lua["package"]["preload"]["events"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        sol::table events = lua.create_table();

        events["on"] = [](sol::this_state s, const std::string& name, const sol::function& callback)
        {
            sol::state_view lua{s};
            const auto options = lua.globals()["__load_opts"].get<const PluginLoadOptions&>();

            if (name == "session_stats")
            {
                auto connection = options.sessions.OnSessionStats(
                    [cb = callback](const std::string& session, const lt::span<const int64_t>& counters)
                    {
                        try
                        {
                            std::vector<int64_t> c;
                            for (const int64_t counter : counters) { c.emplace_back(counter); }
                            cb(session, c);
                        }
                        catch (const sol::error& err)
                        {
                            BOOST_LOG_TRIVIAL(error) << "An error occurred in an event handler: " << err.what();
                        }
                    });

                return std::make_shared<SignalConnection>(connection);
            }

            if (name == "torrent_added")
            {
                auto connection = options.sessions.OnTorrentAdded(
                    [cb = callback](const std::string& session, const lt::torrent_handle& th)
                    {
                        try
                        {
                            cb(th);
                        }
                        catch (const sol::error& err)
                        {
                            BOOST_LOG_TRIVIAL(error) << "An error occurred in an event handler: " << err.what();
                        }
                    });

                return std::make_shared<SignalConnection>(connection);
            }

            if (name == "torrent_file_error")
            {
                auto connection = options.sessions.OnTorrentFileError(
                    [cb = callback](const std::string& session, const porla::Sessions::TorrentFileErrorEvent& evt)
                    {
                        try
                        {
                            cb(evt.torrent, evt.file);
                        }
                        catch (const sol::error& err)
                        {
                            BOOST_LOG_TRIVIAL(error) << "An error occurred in an event handler: " << err.what();
                        }
                    });

                return std::make_shared<SignalConnection>(connection);
            }

            if (name == "torrent_finished")
            {
                auto connection = options.sessions.OnTorrentFinished(
                    [cb = callback](const std::string& session, const lt::torrent_handle& th)
                    {
                        try
                        {
                            cb(th);
                        }
                        catch (const sol::error& err)
                        {
                            BOOST_LOG_TRIVIAL(error) << "An error occurred in an event handler: " << err.what();
                        }
                    });

                return std::make_shared<SignalConnection>(connection);
            }

            if (name == "torrent_moved")
            {
                auto connection = options.sessions.OnStorageMoved(
                    [cb = callback](const std::string& session, const lt::torrent_handle& th)
                    {
                        try
                        {
                            cb(th);
                        }
                        catch (const sol::error& err)
                        {
                            BOOST_LOG_TRIVIAL(error) << "An error occurred in an event handler: " << err.what();
                        }
                    });

                return std::make_shared<SignalConnection>(connection);
            }

            if (name == "torrent_paused")
            {
                auto connection = options.sessions.OnTorrentPaused(
                    [cb = callback](const std::string& session, const lt::torrent_handle& th)
                    {
                        try
                        {
                            cb(th);
                        }
                        catch (const sol::error& err)
                        {
                            BOOST_LOG_TRIVIAL(error) << "An error occurred in an event handler: " << err.what();
                        }
                    });

                return std::make_shared<SignalConnection>(connection);
            }

            if (name == "torrent_removed")
            {
                auto connection = options.sessions.OnTorrentRemoved(
                    [cb = callback](const std::string& session, const lt::info_hash_t& ih)
                    {
                        try
                        {
                            cb(ih);
                        }
                        catch (const sol::error& err)
                        {
                            BOOST_LOG_TRIVIAL(error) << "An error occurred in an event handler: " << err.what();
                        }
                    });

                return std::make_shared<SignalConnection>(connection);
            }

            if (name == "torrent_resumed")
            {
                auto connection = options.sessions.OnTorrentResumed(
                    [cb = callback](const std::string& session, const lt::torrent_handle& th)
                    {
                        try
                        {
                            cb(th);
                        }
                        catch (const sol::error& err)
                        {
                            BOOST_LOG_TRIVIAL(error) << "An error occurred in an event handler: " << err.what();
                        }
                    });

                return std::make_shared<SignalConnection>(connection);
            }

            return std::shared_ptr<SignalConnection>();
        };

        return events;
    };
}
