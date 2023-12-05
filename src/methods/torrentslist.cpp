#include "torrentslist.hpp"

#include "../query/pql.hpp"
#include "../sessions.hpp"
#include "../torrentclientdata.hpp"
#include "../utils/eta.hpp"
#include "../utils/ratio.hpp"

using porla::Methods::TorrentsList;

TorrentsList::TorrentsList(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsList::Invoke(const TorrentsListReq& req, WriteCb<TorrentsListRes> cb)
{
    static std::map<std::pair<std::string, bool>, std::function<bool(const TorrentsListRes::Item&, const TorrentsListRes::Item&)>> sorters =
    {
        {{"download_rate", false},  [](auto const& lhs, auto const& rhs) { return lhs.download_rate > rhs.download_rate; }},
        {{"download_rate", true},   [](auto const& lhs, auto const& rhs) { return lhs.download_rate < rhs.download_rate; }},
        {
            {"eta", false},
            [](auto const& lhs, auto const& rhs)
            {
                if (lhs.eta < 0) return false;
                if (rhs.eta < 0) return true;
                return lhs.eta > rhs.eta;
            }
        },
        {
            {"eta", true},
            [](auto const& lhs, auto const& rhs)
            {
                if (lhs.eta < 0) return false;
                if (rhs.eta < 0) return true;
                return lhs.eta < rhs.eta;
            }
        },
        {{"list_peers", false},     [](auto const& lhs, auto const& rhs) { return lhs.list_peers > rhs.list_peers; }},
        {{"list_peers", true},      [](auto const& lhs, auto const& rhs) { return lhs.list_peers < rhs.list_peers; }},
        {{"list_seeds", false},     [](auto const& lhs, auto const& rhs) { return lhs.list_seeds > rhs.list_seeds; }},
        {{"list_seeds", true},      [](auto const& lhs, auto const& rhs) { return lhs.list_seeds < rhs.list_seeds; }},
        {{"name", false},           [](auto const& lhs, auto const& rhs) { return strcmp(lhs.name.c_str(), rhs.name.c_str()) > 0; }},
        {{"name", true},            [](auto const& lhs, auto const& rhs) { return strcmp(lhs.name.c_str(), rhs.name.c_str()) < 0; }},
        {{"num_peers", false},      [](auto const& lhs, auto const& rhs) { return lhs.num_peers > rhs.num_peers; }},
        {{"num_peers", true},       [](auto const& lhs, auto const& rhs) { return lhs.num_peers < rhs.num_peers; }},
        {{"num_seeds", false},      [](auto const& lhs, auto const& rhs) { return lhs.num_seeds > rhs.num_seeds; }},
        {{"num_seeds", true},       [](auto const& lhs, auto const& rhs) { return lhs.num_seeds < rhs.num_seeds; }},
        {{"progress", false},       [](auto const& lhs, auto const& rhs) { return lhs.progress > rhs.progress; }},
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
        {{"ratio", false},          [](auto const& lhs, auto const& rhs) { return lhs.ratio > rhs.ratio; }},
        {{"ratio", true},           [](auto const& lhs, auto const& rhs) { return lhs.ratio < rhs.ratio; }},
        {{"save_path", false},      [](auto const& lhs, auto const& rhs) { return strcmp(lhs.save_path.c_str(), rhs.save_path.c_str()) > 0; }},
        {{"save_path", true},       [](auto const& lhs, auto const& rhs) { return strcmp(lhs.save_path.c_str(), rhs.save_path.c_str()) < 0; }},
        {{"size", false},           [](auto const& lhs, auto const& rhs) { return lhs.size > rhs.size; }},
        {{"size", true},            [](auto const& lhs, auto const& rhs) { return lhs.size < rhs.size; }},
        {{"total", false},          [](auto const& lhs, auto const& rhs) { return lhs.total > rhs.total; }},
        {{"total", true},           [](auto const& lhs, auto const& rhs) { return lhs.total < rhs.total; }},
        {{"total_done", false},     [](auto const& lhs, auto const& rhs) { return lhs.total_done > rhs.total_done; }},
        {{"total_done", true},      [](auto const& lhs, auto const& rhs) { return lhs.total_done < rhs.total_done; }},
        {{"upload_rate", false},    [](auto const& lhs, auto const& rhs) { return lhs.upload_rate > rhs.upload_rate; }},
        {{"upload_rate", true},     [](auto const& lhs, auto const& rhs) { return lhs.upload_rate < rhs.upload_rate; }},
    };

    std::string field = req.order_by.value_or("queue_position");
    bool order_asc = req.order_by_dir.value_or("asc") == "asc";

    auto const& sorter = sorters.find({field,order_asc});

    if (sorter == sorters.end())
    {
        return cb.Error(-1, "Invalid field in 'order_by'");
    }

    const int global_total_torrents = std::accumulate(
        m_sessions.All().begin(),
        m_sessions.All().end(),
        0,
        [](int current, const auto& state)
        {
            return current + state.second->torrents.size();
        });

    std::vector<TorrentsListRes::Item> torrents;
    torrents.reserve(m_sessions.Default()->torrents.size());

    for (const auto& [ name, state] : m_sessions.All())
    {
        for (auto const& [_, handle] : state->torrents)
        {
            if (!handle.is_valid())
            {
                continue;
            }

            const auto client_data = handle.userdata().get<TorrentClientData>();

            std::map<std::string, json> metadata = {};
            std::int64_t size                    = -1;

            if (req.include_metadata.has_value())
            {
                const auto metadata_keys   = req.include_metadata.value();
                const auto metadata_client = client_data->metadata.value_or(std::map<std::string, nlohmann::json>());

                // Include metadata for all the keys specified. If ["*"], include everything.

                if (metadata_keys.size() == 1 && metadata_keys.at(0) == "*")
                {
                    metadata = metadata_client;
                }
                else
                {
                    for (const auto& key : metadata_keys)
                    {
                        if (!metadata_client.contains(key)) continue;
                        metadata[key] = metadata_client.at(key);
                    }
                }
            }

            auto const& ts = handle.status();

            if (auto ti = ts.torrent_file.lock())
                size = ti->total_size();

            // Filter torrents here.
            bool filter_includes_torrent = true;

            if (const auto& filters = req.filters)
            {
                for (const auto& [filter_field, args] : filters.value())
                {
                    if (filter_field == "category" && args.is_string())
                    {
                        filter_includes_torrent = client_data->category == args.get<std::string>();
                    }
                    else if (filter_field == "query" && args.is_string() && !args.get<std::string>().empty())
                    {
                        try
                        {
                            const auto filter = Query::PQL::Parse(args.get<std::string>());
                            filter_includes_torrent = filter->Includes(ts);
                        }
                        catch (const Query::QueryError& qe)
                        {
                            return cb.Error(-1000, qe.what(), {{"pos", qe.pos()}});
                        }
                    }
                    else if (filter_field == "save_path" && args.is_string())
                    {
                        filter_includes_torrent = ts.save_path == args.get<std::string>();
                    }
                    else if (filter_field == "session" && args.is_string())
                    {
                        filter_includes_torrent = name == args.get<std::string>();
                    }
                    else if (filter_field == "tags" && args.is_string())
                    {
                        const auto& tag_value = args.get<std::string>();
                        const auto  tags      = client_data->tags;

                        filter_includes_torrent = tags.find(tag_value) != tags.end();
                    }
                }
            }

            if (!filter_includes_torrent)
            {
                continue;
            }

#define INSERT_FLAG(name) if ((ts.flags & lt::torrent_flags:: name) == lt::torrent_flags:: name) flags.emplace_back(#name);

            std::vector<std::string> flags;
            INSERT_FLAG(seed_mode)
            INSERT_FLAG(upload_mode)
            INSERT_FLAG(share_mode)
            INSERT_FLAG(apply_ip_filter)
            INSERT_FLAG(paused)
            INSERT_FLAG(auto_managed)
            INSERT_FLAG(duplicate_is_error)
            INSERT_FLAG(update_subscribe)
            INSERT_FLAG(super_seeding)
            INSERT_FLAG(sequential_download)
            INSERT_FLAG(stop_when_ready)
            INSERT_FLAG(need_save_resume)
            INSERT_FLAG(disable_dht)
            INSERT_FLAG(disable_lsd)
            INSERT_FLAG(disable_pex)
            INSERT_FLAG(no_verify_files)
            INSERT_FLAG(default_dont_download)
            INSERT_FLAG(i2p_torrent)

            torrents.emplace_back(TorrentsListRes::Item{
                .active_duration   = ts.active_duration.count(),
                .all_time_download = ts.all_time_download,
                .all_time_upload   = ts.all_time_upload,
                .category          = client_data->category,
                .download_rate     = ts.download_rate,
                .error             = ts.errc,
                .eta               = porla::Utils::ETA(ts).count(),
                .finished_duration = ts.finished_duration.count(),
                .flags             = flags,
                .info_hash         = ts.info_hashes,
                .last_download     = ts.last_download.time_since_epoch().count() > 0
                    ? lt::total_seconds(lt::clock_type::now() - ts.last_download)
                    : -1,
                .last_upload       = ts.last_upload.time_since_epoch().count() > 0
                    ? lt::total_seconds(lt::clock_type::now() - ts.last_upload)
                    : -1,
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
                .seeding_duration  = ts.seeding_duration.count(),
                .session           = name,
                .size              = size,
                .state             = ts.state,
                .tags              = client_data->tags,
                .total             = ts.total,
                .total_done        = ts.total_done,
                .upload_rate       = ts.upload_rate,
            });
        }
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
        .order_by                  = req.order_by.value_or("queue_position"),
        .order_by_dir              = req.order_by_dir.value_or("asc"),
        .page                      = req.page.value_or(0),
        .page_size                 = req.page_size.value_or(50),
        .torrents                  = std::vector(torrents.begin() + page_beg, torrents.begin() + page_end),
        .torrents_total            = static_cast<int>(torrents.size()),
        .torrents_total_unfiltered = static_cast<int>(global_total_torrents)
    });
}
