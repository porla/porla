#include "torrentslist.hpp"

#include "../query/pql.hpp"
#include "../sessions.hpp"
#include "../torrentclientdata.hpp"
#include "../utils/eta.hpp"
#include "../utils/ratio.hpp"

using porla::Methods::TorrentsList;

namespace {
using FilterHashList = std::vector<std::pair<std::uint64_t, nlohmann::json>>;
using Item = porla::Methods::TorrentsListRes::Item;
using SortFn = bool(*)(Item const&, Item const&);

enum SortOrder : std::uint8_t {
  kOrder_Asc  = 0,
  kOrder_Desc = 1
};

/**
 * Hash a C-string using DJB2 xor-variant hash.
 * This is very fast for small strings. Much faster than plain string comparison.
 * @param [in] sz C-String to hash
 * @param [in] sort_order Based on ASC/DESC this will set the most-significant/leftmost bit
 * so we can easily differentiate cases without branching.
 * @return DJB2 hash of the input string.
 */
__attribute__((always_inline)) inline
constexpr std::uint64_t HashDJB2(const char* sz, SortOrder sort_order = kOrder_Asc) {
    std::uint32_t ch, hash = 5381u;
    while ((ch = static_cast<std::uint8_t>(*sz++)))
        hash = (hash * 33u) ^ ch;

    // Set most significant bit (for asc/desc sorting)
    // 00000000 00000000 00000000 00000000 00000000 00000100 00000000 00000000
    // 10000000 00000000 00000000 00000000 00000000 00000100 00000000 00000000
    return std::uint64_t(hash) | std::uint64_t(sort_order) << 63;
}

__attribute__((always_inline)) inline
constexpr std::uint64_t operator"" _djb2(const char *s, unsigned long)
{
    return HashDJB2(s);
}

__attribute__((always_inline)) inline
bool ShouldFilterOut(
    const FilterHashList& filters,
    const libtorrent::torrent_status& ts,
    const std::string& session_name,
    porla::TorrentClientData* client_data
) {
    bool filter_includes_torrent = true;

    for (const auto& [field_hash, args] : filters)
    {
        // Stop early if this torrent can't conform to current filter
        if (!filter_includes_torrent)
        {
            break;
        }

        if (!args.is_string())
        {
            continue;
        }

        switch (field_hash)
        {
        case "category"_djb2:
        {
            filter_includes_torrent &= client_data->category == args.get<std::string>();
            break;
        }
        case "query"_djb2:
        {
            if (!args.get<std::string>().empty()) {
                const auto parsed = porla::Query::PQL::Parse(args.get<std::string>());
                filter_includes_torrent &= parsed->Includes(ts);
            }
            break;
        }
        case "save_path"_djb2:
        {
            filter_includes_torrent &= ts.save_path == args.get<std::string>();
            break;
        }
        case "session"_djb2:
        {
            filter_includes_torrent &= session_name == args.get<std::string>();
            break;
        }
        case "tags"_djb2:
        {
            const auto& tag_value = args.get<std::string>();
            const auto& tags      = client_data->tags;

            filter_includes_torrent &= tags.find(tag_value) != tags.end();
            break;
        }
        default:
            break;
        }
    }
    return filter_includes_torrent;
}

__attribute__((always_inline)) inline
SortFn GetSortComparator(
    const std::string& order_by,
    const std::string& order_by_dir
) {
#define NUMBER_SORT(FIELD) \
    static const auto FIELD##_ASC  = [](Item const& lhs, Item const& rhs) { return lhs.FIELD > rhs.FIELD; }; \
    static const auto FIELD##_DESC = [](Item const& lhs, Item const& rhs) { return lhs.FIELD < rhs.FIELD; };

    NUMBER_SORT(download_rate)
    NUMBER_SORT(list_peers)
    NUMBER_SORT(list_seeds)
    NUMBER_SORT(num_peers)
    NUMBER_SORT(num_seeds)
    NUMBER_SORT(progress)
    NUMBER_SORT(ratio)
    NUMBER_SORT(size)
    NUMBER_SORT(total)
    NUMBER_SORT(total_done)
    NUMBER_SORT(upload_rate)
#undef NUMBER_SORT // !NUMBER_SORT

#define PRIORITY_SORT(FIELD) \
    static const auto FIELD##_ASC  = [](Item const& lhs, Item const& rhs) \
    { \
        if (lhs.FIELD < 0) return false; \
        else if (rhs.FIELD < 0) return true; \
        else return lhs.FIELD > rhs.FIELD; \
    }; \
    static const auto FIELD##_DESC = [](Item const& lhs, Item const& rhs) \
    { \
        if (lhs.FIELD < 0) return false; \
        else if (rhs.FIELD < 0) return true; \
        else return lhs.FIELD < rhs.FIELD; \
    };

    PRIORITY_SORT(eta)
    PRIORITY_SORT(queue_position)
#undef PRIORITY_SORT // !PRIORITY_SORT

    static const auto name_ASC = [](Item const& lhs, Item const& rhs)
    {
        return strcmp(lhs.name.c_str(), rhs.name.c_str()) > 0;
    };

    static const auto name_DESC = [](Item const& lhs, Item const& rhs)
    {
        return strcmp(lhs.name.c_str(), rhs.name.c_str()) < 0;
    };

    const auto field_hash = HashDJB2(
        order_by.c_str(),
        order_by_dir == "asc" ? kOrder_Asc : kOrder_Desc
    );

#define USE_SORT(FIELD)                                                     \
    case (HashDJB2(#FIELD, kOrder_Asc)):  { return FIELD##_ASC; break; }  \
    case (HashDJB2(#FIELD, kOrder_Desc)): { return FIELD##_DESC; break; } \

    switch (field_hash)
    {
    USE_SORT(download_rate)
    USE_SORT(list_peers)
    USE_SORT(list_seeds)
    USE_SORT(num_peers)
    USE_SORT(num_seeds)
    USE_SORT(progress)
    USE_SORT(ratio)
    USE_SORT(size)
    USE_SORT(total)
    USE_SORT(total_done)
    USE_SORT(upload_rate)
    USE_SORT(eta)
    USE_SORT(queue_position)
    USE_SORT(name)
    default:
        return nullptr;
    }
}
}

TorrentsList::TorrentsList(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsList::InvokeImpl(const TorrentsListReq& req, WriteCb<TorrentsListRes>& cb)
{
    std::uintptr_t global_total_torrents = 0;

    std::vector<TorrentsListRes::Item> torrents;
    torrents.reserve(m_sessions.Default()->torrents.size());

    const auto now = lt::clock_type::now();

    FilterHashList filters{};
    if (req.filters.has_value())
    {
        filters.reserve(req.filters->size());

        for (const auto& [key, value] : *req.filters)
        {
            filters.emplace_back(HashDJB2(key.c_str()), value);
        }
    }

    for (const auto& [session_name, state] : m_sessions.All())
    {
        global_total_torrents += state->torrents.size();

        for (auto const& [_, handle] : state->torrents)
        {
            if (!handle.is_valid())
            {
                continue;
            }

            const auto client_data = handle.userdata().get<TorrentClientData>();

            // Filter torrents here.
            auto const& ts = handle.status();

            if (!ShouldFilterOut(filters, ts, session_name, client_data))
            {
                continue;
            }

            std::unordered_map<std::string, nlohmann::json> metadata = {};

            if (req.include_metadata.has_value())
            {
                const auto metadata_keys   = req.include_metadata.value();
                const auto metadata_client = client_data->metadata.value_or(std::map<std::string, nlohmann::json>{});

                // Include metadata for all the keys specified. If ["*"], include everything.
                if (metadata_keys.size() == 1 && !metadata_keys[0].empty() && metadata_keys[0][0] == '*')
                {
                    // Copy all metadata fields from std::map to std::unordered_map
                    metadata = std::unordered_map<std::string, nlohmann::json>{
                        metadata_client.begin(),
                        metadata_client.end()
                    };
                }
                else
                {
                    // Iterate over requested metadata fields
                    // and only include relevant ones
                    for (const auto& key : metadata_keys)
                    {
                        auto it = metadata_client.find(key);
                        if (it == metadata_client.end())
                        {
                            continue;
                        }
                        else
                        {
                            metadata[key] = *it;
                        }
                    }
                }
            }

#define INSERT_FLAG(name) if ((ts.flags & lt::torrent_flags:: name) == lt::torrent_flags:: name) flags.emplace_back(#name);

            // Assume at least some allocation
            // Is this usually the case though?
            std::vector<std::string> flags;
            flags.reserve(4);

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
            INSERT_FLAG(override_trackers)
            INSERT_FLAG(override_web_seeds)
            INSERT_FLAG(need_save_resume)
            INSERT_FLAG(disable_dht)
            INSERT_FLAG(disable_lsd)
            INSERT_FLAG(disable_pex)
            INSERT_FLAG(no_verify_files)
            INSERT_FLAG(default_dont_download)
            INSERT_FLAG(i2p_torrent)

#undef INSERT_FLAG // !INSERT_FLAG

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
                    ? lt::total_seconds(now - ts.last_download)
                    : -1,
                .last_upload       = ts.last_upload.time_since_epoch().count() > 0
                    ? lt::total_seconds(now - ts.last_upload)
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
                .session           = session_name,
                .size              = ts.total,
                .state             = ts.state,
                .tags              = client_data->tags,
                .total             = ts.total,
                .total_done        = ts.total_done,
                .upload_rate       = ts.upload_rate,
            });
        }
    }

    const auto page = req.page.value_or(0);
    const auto page_size = req.page_size.value_or(50);

    const auto page_beg = page * page_size;
    const auto page_end = std::min(
        page_beg + page_size,
        static_cast<int>(torrents.size())
    );

    if (page_beg > page_end)
    {
        return cb.Error(-2, "Invalid list range: page begin > end.");
    }

    const auto order_by = req.order_by.value_or("queue_position");
    const auto order_by_dir = req.order_by_dir.value_or("asc");

    auto* sort_fn = GetSortComparator(order_by, order_by_dir);
    if (!sort_fn)
    {
        return cb.Error(-1, "Invalid field in 'order_by'");
    }

    // Keep top `page_size` entries sorted at the beginning of the list.
    // Iterates through the entire list.
    std::partial_sort(torrents.begin(), torrents.begin() + page_end, torrents.end(), sort_fn);

    cb.Ok(TorrentsListRes{
        .order_by                  = order_by,
        .order_by_dir              = order_by_dir,
        .page                      = page,
        .page_size                 = page_size,
        .torrents                  = std::vector(torrents.begin() + page_beg, torrents.begin() + page_end),
        .torrents_total            = static_cast<int>(torrents.size()),
        .torrents_total_unfiltered = static_cast<int>(global_total_torrents)
    });
}

void TorrentsList::Invoke(const TorrentsListReq& req, WriteCb<TorrentsListRes> cb) {
    try
    {
        InvokeImpl(req, cb);
    }
    catch (const Query::QueryError& qe)
    {
        return cb.Error(-1000, qe.what(), {{"pos", qe.pos()}});
    }
}