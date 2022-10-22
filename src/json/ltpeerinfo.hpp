#pragma once

#include <libtorrent/peer_info.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace libtorrent
{
    static void to_json(json& j, const libtorrent::peer_info& pi)
    {
        j = {
            {"busy_requests", pi.busy_requests},
            {"client", pi.client},
            {"connection_type", static_cast<uint8_t>(pi.connection_type)},
            {"down_speed", pi.down_speed},
            {"download_queue_length", pi.download_queue_length},
            {"download_queue_time", pi.download_queue_time.count()},
            {"flags", static_cast<uint32_t>(pi.flags)},
            {"ip", {
                pi.ip.address().to_string(),
                pi.ip.port()
            }},
            {"last_active", pi.last_active.count()},
            {"last_request", pi.last_request.count()},
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
