#include "../all.hpp"

#include <libtorrent/peer_info.hpp>

namespace libtorrent
{
    void to_json(nlohmann::json& json, const peer_info& pi)
    {
        json = {
            {"busy_requests", pi.busy_requests},
            {"client", pi.client},
            {"connection_type", static_cast<uint8_t>(pi.connection_type)},
            {"down_speed", pi.down_speed},
            {"download_queue_length", pi.download_queue_length},
            {"download_queue_time", libtorrent::total_seconds(pi.download_queue_time)},
            {"flags", static_cast<uint32_t>(pi.flags)},
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
            {"source", static_cast<uint8_t>(pi.source)},
            {"total_download", pi.total_download},
            {"total_upload", pi.total_upload},
            {"up_speed", pi.up_speed},
        };
    }
}
