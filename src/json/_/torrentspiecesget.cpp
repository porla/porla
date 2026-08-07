#include "../all.hpp"

#include "../../methods/torrents/torrentspiecesget_reqres.hpp"
#include "../../utils/base64.hpp"
#include "../utils.hpp"

namespace libtorrent
{
    void from_json(const nlohmann::json& j, typed_bitfield<piece_index_t>& pieces)
    {
    }

    void to_json(nlohmann::json& j, const typed_bitfield<piece_index_t>& pieces)
    {
        std::string bytes;

        if (pieces.data() != nullptr && pieces.num_bytes() > 0)
        {
            bytes.assign(pieces.data(), static_cast<std::size_t>(pieces.num_bytes()));
        }

        j = { pieces.size(), porla::Utils::Base64::Encode(bytes) };
    }
}

namespace porla::Methods
{
    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsPiecesGetReq,
        info_hash,
        session_id)

    NLOHMANN_JSONIFY_ALL_THINGS(
        TorrentsPiecesGetRes,
        pieces,
        verified_pieces)
}
