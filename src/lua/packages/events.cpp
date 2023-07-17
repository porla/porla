#include "events.hpp"

#include <boost/log/trivial.hpp>

#include "../plugins/plugin.hpp"
#include "../../session.hpp"

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
    lua["package"]["preload"]["events"] = [](sol::this_state s)
    {
        sol::state_view lua{s};
        sol::table events = lua.create_table();

        events["on"] = [](sol::this_state s, const std::string& name, const sol::function& callback)
        {
            sol::state_view lua{s};
            const auto options = lua.globals()["__load_opts"].get<const Plugins::PluginLoadOptions&>();

            if (name == "torrent_added")
            {
                auto connection = options.session.OnTorrentAdded(
                    [cb = callback](const lt::torrent_status& ts)
                    {
                        try
                        {
                            cb(ts);
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
                auto connection = options.session.OnTorrentFinished(
                    [cb = callback](const lt::torrent_status& ts)
                    {
                        try
                        {
                            cb(ts);
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
                auto connection = options.session.OnStorageMoved(
                    [cb = callback](const lt::torrent_handle& th)
                    {
                        try
                        {
                            cb(th.status());
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
                auto connection = options.session.OnTorrentPaused(
                    [cb = callback](const lt::torrent_handle& th)
                    {
                        try
                        {
                            cb(th.status());
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
                auto connection = options.session.OnTorrentRemoved(
                    [cb = callback](const lt::info_hash_t& ih)
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
                auto connection = options.session.OnTorrentResumed(
                    [cb = callback](const lt::torrent_status& ts)
                    {
                        try
                        {
                            cb(ts);
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
