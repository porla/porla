#include "../all.hpp"

namespace libtorrent
{
    void to_json(nlohmann::json& json, const dht_lookup& lookup)
    {
        std::stringstream target_hex;
        target_hex << lookup.target;

        json = nlohmann::json::object();
        json["type"] = lookup.type;
        json["outstanding_requests"] = lookup.outstanding_requests;
        json["timeouts"] = lookup.timeouts;
        json["responses"] = lookup.responses;
        json["branch_factor"] = lookup.branch_factor;
        json["nodes_left"] = lookup.nodes_left;
        json["last_sent"] = lookup.last_sent;
        json["first_timeout"] = lookup.first_timeout;
        json["target"] = target_hex.str();
    }

    void to_json(nlohmann::json& json, const dht_routing_bucket& bucket)
    {
        json = nlohmann::json::object();
        json["num_nodes"] = bucket.num_nodes;
        json["num_replacements"] = bucket.num_replacements;
        json["last_active"] = bucket.last_active;
    }

    void to_json(nlohmann::json& json, const add_torrent_alert& alert)
    {
        json = nlohmann::json::object();
        json["error"] = alert.error;
    }

    void to_json(nlohmann::json& json, const dht_stats_alert& alert)
    {
        std::stringstream nid_hex;
        nid_hex << alert.nid;

        json = nlohmann::json::object();
        json["active_requests"] = alert.active_requests;
        json["routing_table"] = alert.routing_table;
        json["nid"] = nid_hex.str();
        json["local_endpoint"] = {
            alert.local_endpoint.address().to_string(),
            alert.local_endpoint.port()
        };
    }

    void to_json(nlohmann::json& json, const file_error_alert& alert)
    {
        json = nlohmann::json::object();
        json["error"] = alert.error;
        json["op"] = lt::operation_name(alert.op);
        json["filename"] = alert.filename();
    }

    void to_json(nlohmann::json& json, const metadata_received_alert& alert)
    {
        json = nlohmann::json::object();
    }

    void to_json(nlohmann::json& json, const state_changed_alert& alert)
    {
        json = nlohmann::json::object();
        json["state"] = alert.state;
        json["prev_state"] = alert.prev_state;
    }

    void to_json(nlohmann::json& json, const storage_moved_alert& alert)
    {
        json = nlohmann::json::object();
        json["storage_path"] = alert.storage_path();
        json["old_path"] = alert.old_path();
    }

    void to_json(nlohmann::json& json, const storage_moved_failed_alert& alert)
    {
        json = nlohmann::json::object();
        json["error"] = alert.error;
        json["file_path"] = alert.file_path();
        json["op"] = lt::operation_name(alert.op);
    }

    void to_json(nlohmann::json& json, const torrent_checked_alert& alert)
    {
        json = nlohmann::json::object();
    }

    void to_json(nlohmann::json& json, const torrent_error_alert& alert)
    {
        json = nlohmann::json::object();
        json["error"] = alert.error;
        json["filename"] = alert.filename();
    }

    void to_json(nlohmann::json& json, const torrent_finished_alert& alert)
    {
        json = nlohmann::json::object();
    }

    void to_json(nlohmann::json& json, const torrent_need_cert_alert& alert)
    {
        json = nlohmann::json::object();
    }

    void to_json(nlohmann::json& json, const torrent_paused_alert& alert)
    {
        json = nlohmann::json::object();
    }

    void to_json(nlohmann::json& json, const torrent_resumed_alert& alert)
    {
        json = nlohmann::json::object();
    }

    void to_json(nlohmann::json& json, const tracker_error_alert& alert)
    {
        json = nlohmann::json::object();
        json["local_endpoint"] = {
            alert.local_endpoint.address().to_string(),
            alert.local_endpoint.port()
        };
        json["tracker_url"] = alert.tracker_url();
        json["times_in_row"] = alert.times_in_row;
        json["error"] = alert.error;
        json["op"] = lt::operation_name(alert.op);
        json["failure_reason"] = alert.failure_reason();
    }

    void to_json(nlohmann::json& json, const tracker_reply_alert& alert)
    {
        json = nlohmann::json::object();
        json["local_endpoint"] = {
            alert.local_endpoint.address().to_string(),
            alert.local_endpoint.port()
        };
        json["tracker_url"] = alert.tracker_url();
        json["num_peers"] = alert.num_peers;
    }

    void to_json(nlohmann::json& json, const tracker_warning_alert& alert)
    {
        json = nlohmann::json::object();
        json["local_endpoint"] = {
            alert.local_endpoint.address().to_string(),
            alert.local_endpoint.port()
        };
        json["tracker_url"] = alert.tracker_url();
        json["warning_message"] = alert.warning_message();
    }
}
