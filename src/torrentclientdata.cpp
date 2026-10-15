#include "torrentclientdata.hpp"

namespace porla
{
    void from_json(const nlohmann::json& j, TorrentClientData& d)
    {
        if (j.contains("category") && j.at("category").is_string())
        {
            d.category = j["category"];
        }

        if (j.contains("completed_at") && j.at("completed_at").is_number_integer())
        {
            d.completed_at = j["completed_at"].get<std::int64_t>();
        }

        if (j.contains("metadata") && j.at("metadata").is_object())
        {
            d.metadata = j["metadata"];
        }

        if (j.contains("tags") && j.at("tags").is_array())
        {
            d.tags = j["tags"];
        }
    }

    void to_json(nlohmann::json& j, const TorrentClientData& d)
    {
        j = nlohmann::json::object();
        j["category"]     = d.category;
        j["completed_at"] = d.completed_at;
        j["metadata"]     = d.metadata;
        j["tags"]         = d.tags;
    }
}
