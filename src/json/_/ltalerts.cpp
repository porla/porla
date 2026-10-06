#include "../all.hpp"

namespace libtorrent
{
    void to_json(nlohmann::json& json, const add_torrent_alert& alert)
    {
        json = nlohmann::json::object();
        json["error"] = alert.error;
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
