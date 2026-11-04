#include "../types.hpp"

#include <libtorrent/torrent_handle.hpp>

using porla::Lua::Types::LtBlockInfo;

void LtBlockInfo::Register(sol::state& lua)
{
    lua.new_usertype<lt::block_info>(
        "LtBlockInfo",
        sol::no_constructor,
        "state", sol::readonly_property([](const lt::block_info& bi)
        {
            static constexpr const char* kStates[] = {"none", "requested", "writing", "finished"};
            return kStates[bi.state];
        }),
        "bytes_progress", sol::readonly_property([](const lt::block_info& bi) { return static_cast<int>(bi.bytes_progress); }),
        "block_size",     sol::readonly_property([](const lt::block_info& bi) { return static_cast<int>(bi.block_size); }),
        "num_peers",      sol::readonly_property([](const lt::block_info& bi) { return static_cast<int>(bi.num_peers); }),
        "peer",           sol::readonly_property([](const lt::block_info& bi, sol::this_state s) -> sol::object
        {
            const auto peer = bi.peer();

            // libtorrent sets 0.0.0.0:0 when no peer has the block
            if (peer.address().is_unspecified())
            {
                return sol::nil;
            }

            sol::state_view lua(s);

            return lua.create_table_with(1, peer.address().to_string(), 2, peer.port());
        }));
}
