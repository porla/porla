#include "../all.hpp"

#include <sodium.h>

#include "../../rpc/methods/torrents/torrentspiecesget_reqres.hpp"
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

        const auto len = sodium_base64_ENCODED_LEN(bytes.size(), sodium_base64_VARIANT_ORIGINAL);

        std::string output;
        output.resize(len);

        sodium_bin2base64(
            output.data(),
            output.size(),
            reinterpret_cast<const unsigned char*>(bytes.c_str()),
            bytes.size(),
            sodium_base64_VARIANT_ORIGINAL);

        output.resize(len - 1);

        j = { pieces.size(), output };
    }
}

namespace porla::Rpc::Methods::Torrents
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
