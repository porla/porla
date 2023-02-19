#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <libtorrent/fwd.hpp>
#include <sol/sol.hpp>

namespace porla::Lua::UserTypes
{
    class Torrent
    {
    public:
        static void Register(sol::state& lua);

        explicit Torrent(const lt::torrent_handle& th);
        ~Torrent();

    private:
        std::optional<std::string> Category();
        std::string Name();
        std::string SavePath();
        std::optional<std::int64_t> Size();
        std::vector<std::string> Tags();

        struct State;
        std::shared_ptr<State> m_state;
    };
}
