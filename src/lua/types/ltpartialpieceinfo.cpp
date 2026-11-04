#include "../types.hpp"

#include <libtorrent/torrent_handle.hpp>

#include "../../sessions/partialpieceinfo.hpp"

using porla::Lua::Types::LtPartialPieceInfo;

void LtPartialPieceInfo::Register(sol::state& lua)
{
    lua.new_usertype<porla::PartialPieceInfo>(
        "LtPartialPieceInfo",
        sol::no_constructor,
        "piece_index",     sol::readonly_property([](const porla::PartialPieceInfo& pi) { return static_cast<int>(pi.piece_index); }),
        "blocks_in_piece", sol::readonly_property([](const porla::PartialPieceInfo& pi) { return static_cast<int>(pi.blocks.size()); }),
        "finished",        sol::readonly(&porla::PartialPieceInfo::finished),
        "writing",         sol::readonly(&porla::PartialPieceInfo::writing),
        "requested",       sol::readonly(&porla::PartialPieceInfo::requested),
        "blocks",          sol::readonly_property([](const porla::PartialPieceInfo& pi) { return sol::as_table(pi.blocks); }));
}
