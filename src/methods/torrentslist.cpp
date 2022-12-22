#include "torrentslist.hpp"

#include "../data/models/torrentsmetadata.hpp"
#include "../session.hpp"
#include "../utils/eta.hpp"
#include "../utils/ratio.hpp"

using porla::Data::Models::TorrentsMetadata;
using porla::Methods::TorrentsList;

TorrentsList::TorrentsList(sqlite3* db, porla::ISession& session)
    : m_db(db)
    , m_session(session)
{
}

void TorrentsList::Invoke(const TorrentsListReq& req, WriteCb<TorrentsListRes> cb)
{
    static std::map<std::pair<std::string, bool>, std::function<bool(const TorrentsListRes::Item&, const TorrentsListRes::Item&)>> sorters =
    {
        {{"download_rate", false},  [](auto const& lhs, auto const& rhs) { return lhs.download_rate >= rhs.download_rate; }},
        {{"download_rate", true},   [](auto const& lhs, auto const& rhs) { return lhs.download_rate < rhs.download_rate; }},
        {{"list_peers", false},     [](auto const& lhs, auto const& rhs) { return lhs.list_peers >= rhs.list_peers; }},
        {{"list_peers", true},      [](auto const& lhs, auto const& rhs) { return lhs.list_peers < rhs.list_peers; }},
        {{"list_seeds", false},     [](auto const& lhs, auto const& rhs) { return lhs.list_seeds >= rhs.list_seeds; }},
        {{"list_seeds", true},      [](auto const& lhs, auto const& rhs) { return lhs.list_seeds < rhs.list_seeds; }},
        {{"name", false},           [](auto const& lhs, auto const& rhs) { return strcmp(lhs.name.c_str(), rhs.name.c_str()) > 0; }},
        {{"name", true},            [](auto const& lhs, auto const& rhs) { return strcmp(lhs.name.c_str(), rhs.name.c_str()) < 0; }},
        {{"num_peers", false},      [](auto const& lhs, auto const& rhs) { return lhs.num_peers >= rhs.num_peers; }},
        {{"num_peers", true},       [](auto const& lhs, auto const& rhs) { return lhs.num_peers < rhs.num_peers; }},
        {{"num_seeds", false},      [](auto const& lhs, auto const& rhs) { return lhs.num_seeds >= rhs.num_seeds; }},
        {{"num_seeds", true},       [](auto const& lhs, auto const& rhs) { return lhs.num_seeds < rhs.num_seeds; }},
        {{"progress", false},       [](auto const& lhs, auto const& rhs) { return lhs.progress >= rhs.progress; }},
        {{"progress", true},        [](auto const& lhs, auto const& rhs) { return lhs.progress < rhs.progress; }},
        {
            {"queue_position", false},
            [](auto const& lhs, auto const& rhs)
            {
                if (lhs.queue_position < 0) return false;
                if (rhs.queue_position < 0) return true;
                return lhs.queue_position >= rhs.queue_position;
            }
        },
        {
            {"queue_position", true},
            [](auto const& lhs, auto const& rhs)
            {
                if (lhs.queue_position < 0) return false;
                if (rhs.queue_position < 0) return true;
                return lhs.queue_position < rhs.queue_position;
            }
        },
        {{"save_path", false},      [](auto const& lhs, auto const& rhs) { return strcmp(lhs.save_path.c_str(), rhs.save_path.c_str()) > 0; }},
        {{"save_path", true},       [](auto const& lhs, auto const& rhs) { return strcmp(lhs.save_path.c_str(), rhs.save_path.c_str()) < 0; }},
        {{"size", false},           [](auto const& lhs, auto const& rhs) { return lhs.size >= rhs.size; }},
        {{"size", true},            [](auto const& lhs, auto const& rhs) { return lhs.size < rhs.size; }},
        {{"total", false},          [](auto const& lhs, auto const& rhs) { return lhs.total >= rhs.total; }},
        {{"total", true},           [](auto const& lhs, auto const& rhs) { return lhs.total < rhs.total; }},
        {{"total_done", false},     [](auto const& lhs, auto const& rhs) { return lhs.total_done >= rhs.total_done; }},
        {{"total_done", true},      [](auto const& lhs, auto const& rhs) { return lhs.total_done < rhs.total_done; }},
        {{"upload_rate", false},    [](auto const& lhs, auto const& rhs) { return lhs.upload_rate >= rhs.upload_rate; }},
        {{"upload_rate", true},     [](auto const& lhs, auto const& rhs) { return lhs.upload_rate < rhs.upload_rate; }},
    };

    std::string field = req.order_by.value_or("queue_position");
    bool order_asc = req.order_by_dir.value_or("asc") == "asc";

    auto const& sorter = sorters.find({field,order_asc});

    if (sorter == sorters.end())
    {
        return cb.Error(-1, "Invalid field in 'order_by'");
    }

    std::vector<TorrentsListRes::Item> torrents;
    torrents.reserve(m_session.Torrents().size());

    for (auto const& [_, handle] : m_session.Torrents())
    {
        auto const stored_metadata = TorrentsMetadata::GetAll(m_db, handle.info_hashes());

        json category                 = json();
        std::optional<json> metadata  = std::nullopt;
        std::int64_t size             = -1;
        std::vector<std::string> tags = {};

        if (stored_metadata.contains("category")
            && stored_metadata.at("category").is_string())
        {
            category = stored_metadata.at("category");
        }

        if (stored_metadata.contains("tags"))
        {
            stored_metadata.at("tags").get_to(tags);
        }

        if (req.include_metadata.has_value())
        {
            auto const metadata_keys = req.include_metadata.value();

            // Include metadata for all the keys specified. If ["*"], include everything.

            if (metadata_keys.size() == 1 && metadata_keys.at(0) == "*")
            {
                metadata = stored_metadata;
            }
            else
            {
                metadata = json::object({});

                for (auto const& key : metadata_keys)
                {
                    if (!stored_metadata.contains(key)) continue;
                    metadata.value()[key] = stored_metadata.at(key);
                }
            }
        }

        auto const& ts = handle.status();

        if (auto ti = ts.torrent_file.lock())
            size = ti->total_size();

        // Filter torrents here.
        bool filter_includes_torrent = true;

        if (auto filters = req.filters)
        {
            for (auto const& filter : filters.value())
            {
                if (filter.field == "category"
                    && filter.args.is_string())
                {
                    auto const& category_value = filter.args.get<std::string>();
                    filter_includes_torrent = category == filter.args;
                }
                else if (filter.field == "tags"
                    && filter.args.is_string())
                {
                    auto const& tag_value = filter.args.get<std::string>();

                    filter_includes_torrent = std::find(
                        tags.begin(),
                        tags.end(),
                        tag_value) != tags.end();
                }
            }
        }

        if (!filter_includes_torrent)
        {
            continue;
        }

        torrents.push_back(TorrentsListRes::Item{
            .all_time_download = ts.all_time_download,
            .all_time_upload   = ts.all_time_upload,
            .category          = category,
            .download_rate     = ts.download_rate,
            .error             = ts.errc,
            .eta               = porla::Utils::ETA(ts).count(),
            .flags             = static_cast<std::uint64_t>(ts.flags),
            .info_hash         = ts.info_hashes,
            .list_peers        = ts.list_peers,
            .list_seeds        = ts.list_seeds,
            .metadata          = metadata,
            .moving_storage    = ts.moving_storage,
            .name              = ts.name,
            .num_peers         = ts.num_peers,
            .num_seeds         = ts.num_seeds,
            .progress          = ts.progress,
            .queue_position    = static_cast<int>(ts.queue_position),
            .ratio             = porla::Utils::Ratio(ts),
            .save_path         = ts.save_path,
            .size              = size,
            .state             = ts.state,
            .tags              = tags,
            .total             = ts.total,
            .total_done        = ts.total_done,
            .upload_rate       = ts.upload_rate,
        });
    }

    std::sort(
        torrents.begin(),
        torrents.end(),
        [&sorter](auto const& lhs, auto const& rhs)
        {
            return sorter->second(lhs, rhs);
        });

    int page_beg = req.page.value_or(0) * req.page_size.value_or(50);
    int page_end = std::min(
        page_beg + req.page_size.value_or(50),
        static_cast<int>(torrents.size()));

    if (page_beg > torrents.size())
    {
        return cb.Error(-2, "Invalid page - too large.");
    }

    cb.Ok(TorrentsListRes{
        .page = req.page.value_or(0),
        .page_size = req.page_size.value_or(50),
        .torrents = std::vector(
            torrents.begin() + page_beg,
            torrents.begin() + page_end),
        .torrents_total = static_cast<int>(torrents.size())
    });
}
