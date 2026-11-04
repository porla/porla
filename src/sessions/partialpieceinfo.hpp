#pragma once

#include <vector>

#include <libtorrent/torrent_handle.hpp>

namespace porla
{
    // provides a copied view of the lt partial piece info to avoid dangling pointers
    struct PartialPieceInfo
    {
        lt::piece_index_t           piece_index;
        int                         finished;
        int                         writing;
        int                         requested;
        std::vector<lt::block_info> blocks;
    };

    inline std::vector<PartialPieceInfo> CopyDownloadQueue(const std::vector<lt::partial_piece_info>& queue)
    {
        std::vector<PartialPieceInfo> pieces;
        pieces.reserve(queue.size());

        for (const auto& pi : queue)
        {
            pieces.push_back({
                .piece_index = pi.piece_index,
                .finished    = pi.finished,
                .writing     = pi.writing,
                .requested   = pi.requested,
                .blocks      = std::vector(pi.blocks, pi.blocks + pi.blocks_in_piece)
            });
        }

        return pieces;
    }
}
