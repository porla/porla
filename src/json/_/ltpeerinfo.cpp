#include "../all.hpp"

#include <unordered_set>

#include <libtorrent/peer_info.hpp>

namespace libtorrent
{
    void to_json(nlohmann::json& json, const peer_info& pi)
    {
        std::string connection_type;
        if (pi.connection_type == lt::peer_info::standard_bittorrent) connection_type = "standard_bittorrent";
        if (pi.connection_type == lt::peer_info::web_seed)            connection_type = "web_seed";
        if (pi.connection_type == lt::peer_info::http_seed)           connection_type = "http_seed";

        std::unordered_set<std::string> flags;
        if (pi.flags & lt::peer_info::interesting)         flags.insert("interesting");
        if (pi.flags & lt::peer_info::choked)              flags.insert("choked");
        if (pi.flags & lt::peer_info::remote_interested)   flags.insert("remote_interested");
        if (pi.flags & lt::peer_info::remote_choked)       flags.insert("remote_choked");
        if (pi.flags & lt::peer_info::supports_extensions) flags.insert("supports_extensions");
        if (pi.flags & lt::peer_info::outgoing_connection) flags.insert("outgoing_connection");
        if (pi.flags & lt::peer_info::handshake)           flags.insert("handshake");
        if (pi.flags & lt::peer_info::connecting)          flags.insert("connecting");
        if (pi.flags & lt::peer_info::on_parole)           flags.insert("on_parole");
        if (pi.flags & lt::peer_info::seed)                flags.insert("seed");
        if (pi.flags & lt::peer_info::optimistic_unchoke)  flags.insert("optimistic_unchoke");
        if (pi.flags & lt::peer_info::snubbed)             flags.insert("snubbed");
        if (pi.flags & lt::peer_info::upload_only)         flags.insert("upload_only");
        if (pi.flags & lt::peer_info::endgame_mode)        flags.insert("endgame_mode");
        if (pi.flags & lt::peer_info::holepunched)         flags.insert("holepunched");
        if (pi.flags & lt::peer_info::i2p_socket)          flags.insert("i2p_socket");
        if (pi.flags & lt::peer_info::utp_socket)          flags.insert("utp_socket");
        if (pi.flags & lt::peer_info::ssl_socket)          flags.insert("ssl_socket");
        if (pi.flags & lt::peer_info::rc4_encrypted)       flags.insert("rc4_encrypted");
        if (pi.flags & lt::peer_info::plaintext_encrypted) flags.insert("plaintext_encrypted");

        std::unordered_set<std::string> source;
        if (pi.source & lt::peer_info::tracker)     source.insert("tracker");
        if (pi.source & lt::peer_info::dht)         source.insert("dht");
        if (pi.source & lt::peer_info::pex)         source.insert("pex");
        if (pi.source & lt::peer_info::lsd)         source.insert("lsd");
        if (pi.source & lt::peer_info::resume_data) source.insert("resume_data");
        if (pi.source & lt::peer_info::incoming)    source.insert("incoming");

        json = {
            {"busy_requests", pi.busy_requests},
            {"client", pi.client},
            {"connection_type", connection_type},
            {"down_speed", pi.down_speed},
            {"download_queue_length", pi.download_queue_length},
            {"download_queue_time", libtorrent::total_seconds(pi.download_queue_time)},
            {"flags", flags},
            {"ip", {
                pi.ip.address().to_string(),
                pi.ip.port()
            }},
            {"last_active", libtorrent::total_seconds(pi.last_active)},
            {"last_request", libtorrent::total_seconds(pi.last_request)},
            {"local_endpoint", {
                pi.local_endpoint.address().to_string(),
                pi.local_endpoint.port()
            }},
            {"progress", pi.progress},
            {"rtt", pi.rtt},
            {"source", source},
            {"total_download", pi.total_download},
            {"total_upload", pi.total_upload},
            {"up_speed", pi.up_speed},
        };
    }
}
