#include "events.hpp"

#include <boost/signals2.hpp>

#include "../pluginstate.hpp"
#include "../types/pocancellable.hpp"

struct PoCancellableConnection : public porla::Lua::Types::PoCancellable
{
    explicit PoCancellableConnection(boost::signals2::scoped_connection conn, int callback_id, int conn_id)
        : m_conn(std::move(conn))
        , m_callback_id(callback_id)
        , m_conn_id(conn_id)
    {
    }

    void Cancel(sol::this_state ts) override
    {
    }

private:
    boost::signals2::scoped_connection m_conn;
    int m_callback_id;
    int m_conn_id;
};

sol::object porla::Lua::Packages::Events::Load(sol::this_state ts)
{
    sol::state_view lua(ts);

    sol::table tbl = lua.create_table();

    tbl.set_function("on", [](sol::this_state ts, std::string event, sol::protected_function callback) -> std::shared_ptr<Types::PoCancellable>
    {
        sol::state_view lua(ts);

        auto weak = lua.registry()["state"].get<std::weak_ptr<LuaState>>();
        auto state = weak.lock();

        if (state == nullptr)
        {
            return nullptr;
        }

        std::size_t                        callback_id = state->next_id++;
        std::size_t                        conn_id     = state->next_id++;
        boost::signals2::scoped_connection conn;

        if (event == "torrent.added")
        {
            conn = state->sessions.OnTorrentAdded(
                [weak, callback_id](const auto session, const auto& handle)
                {
                    auto state = weak.lock();
                    if (state == nullptr) { return; }

                    auto it = state->callbacks.find(callback_id);
                    if (it == state->callbacks.end()) { return; }

                    it->second(handle);
                });
        }
        else if (event == "torrent.file_error")
        {
            conn = state->sessions.OnTorrentFileError(
                [weak, callback_id](const auto session, const auto& err)
                {
                    auto state = weak.lock();
                    if (state == nullptr) { return; }

                    auto it = state->callbacks.find(callback_id);
                    if (it == state->callbacks.end()) { return; }

                    it->second(err.torrent, err.file);
                });
        }
        else if (event == "torrent.finished")
        {
            conn = state->sessions.OnTorrentFinished(
                [weak, callback_id](const auto session, const auto& handle)
                {
                    auto state = weak.lock();
                    if (state == nullptr) { return; }

                    auto it = state->callbacks.find(callback_id);
                    if (it == state->callbacks.end()) { return; }

                    it->second(handle);
                });
        }
        else if (event == "torrent.paused")
        {
            conn = state->sessions.OnTorrentPaused(
                [weak, callback_id](const auto session, const auto& handle)
                {
                    auto state = weak.lock();
                    if (state == nullptr) { return; }

                    auto it = state->callbacks.find(callback_id);
                    if (it == state->callbacks.end()) { return; }

                    it->second(handle);
                });
        }
        else if (event == "torrent.removed")
        {
            conn = state->sessions.OnTorrentRemoved(
                [weak, callback_id](const auto session, const auto& info_hash)
                {
                    auto state = weak.lock();
                    if (state == nullptr) { return; }

                    auto it = state->callbacks.find(callback_id);
                    if (it == state->callbacks.end()) { return; }

                    it->second(info_hash);
                });
        }
        else if (event == "torrent.resumed")
        {
            conn = state->sessions.OnTorrentResumed(
                [weak, callback_id](const auto session, const auto& handle)
                {
                    auto state = weak.lock();
                    if (state == nullptr) { return; }

                    auto it = state->callbacks.find(callback_id);
                    if (it == state->callbacks.end()) { return; }

                    it->second(handle);
                });
        }
        else if (event == "torrent.storage_moved")
        {
            conn = state->sessions.OnStorageMoved(
                [weak, callback_id](const auto session, const auto& handle)
                {
                    auto state = weak.lock();
                    if (state == nullptr) { return; }

                    auto it = state->callbacks.find(callback_id);
                    if (it == state->callbacks.end()) { return; }

                    it->second(handle);
                });
        }
        else
        {
            return nullptr;
        }

        state->callbacks[callback_id] = callback;

        return std::make_shared<PoCancellableConnection>(std::move(conn), callback_id, conn_id);
    });

    return tbl;
}
