#include "../all.hpp"

#include <libtorrent/announce_entry.hpp>

namespace libtorrent
{
    void to_json(nlohmann::json& json, const libtorrent::announce_infohash& ih)
    {
        json = {
            {"complete_sent", ih.complete_sent},
            {"fails", ih.fails},
            {"last_error", ih.last_error},
            {"message", ih.message},
            {"min_announce", ih.min_announce.time_since_epoch().count()},
            {"next_announce", ih.next_announce.time_since_epoch().count()},
            {"scrape_complete", ih.scrape_complete},
            {"scrape_downloaded", ih.scrape_downloaded},
            {"scrape_incomplete", ih.scrape_incomplete},
            {"start_sent", ih.start_sent},
            {"updating", ih.updating},
        };
    }

    void to_json(nlohmann::json& json, const libtorrent::announce_endpoint& endpoint)
    {
        json = {
            {"enabled", endpoint.enabled},
            {"info_hashes", endpoint.info_hashes},
            {"local_endpoint", {
                endpoint.local_endpoint.address().to_string(),
                endpoint.local_endpoint.port()
            }}
        };
    }

    void to_json(nlohmann::json& json, const libtorrent::announce_entry& entry)
    {
        json = {
            {"endpoints", entry.endpoints},
            {"fail_limit", entry.fail_limit},
            {"source", entry.source},
            {"tier", entry.tier},
            {"trackerid", entry.trackerid},
            {"url", entry.url},
            {"verified", entry.verified},
        };
    }
}
