#include "torrentslist.hpp"

#include "../session.hpp"

using porla::Methods::TorrentsList;

TorrentsList::TorrentsList(porla::ISession& session)
    : m_session(session)
{
}

void TorrentsList::Invoke(const TorrentsListReq& req, WriteCb<TorrentsListRes> cb)
{
    static std::map<std::pair<std::string, bool>, std::function<bool(const TorrentsListRes::Item&, const TorrentsListRes::Item&)>> sorters =
    {
        {{"name", false},           [](auto const& lhs, auto const& rhs) { return strcmp(lhs.name.c_str(), rhs.name.c_str()) > 0; }},
        {{"name", true},            [](auto const& lhs, auto const& rhs) { return strcmp(lhs.name.c_str(), rhs.name.c_str()) < 0; }},
        {{"queue_position", false}, [](auto const& lhs, auto const& rhs) { return lhs.queue_position >= rhs.queue_position; }},
        {{"queue_position", true},  [](auto const& lhs, auto const& rhs) { return lhs.queue_position < rhs.queue_position; }},
        {{"save_path", false},      [](auto const& lhs, auto const& rhs) { return strcmp(lhs.save_path.c_str(), rhs.save_path.c_str()) > 0; }},
        {{"save_path", true},       [](auto const& lhs, auto const& rhs) { return strcmp(lhs.save_path.c_str(), rhs.save_path.c_str()) < 0; }},
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

    for (auto const& [_, ts] : m_session.Torrents())
    {
        std::int64_t size = -1;

        if (auto ti = ts.torrent_file.lock())
            size = ti->total_size();

        torrents.push_back(TorrentsListRes::Item{
            .download_rate  = ts.download_rate,
            .flags          = static_cast<std::uint64_t>(ts.flags),
            .info_hashes    = ts.info_hashes,
            .list_peers     = ts.list_peers,
            .list_seeds     = ts.list_seeds,
            .name           = ts.name,
            .num_peers      = ts.num_peers,
            .num_seeds      = ts.num_seeds,
            .progress       = ts.progress,
            .queue_position = static_cast<int>(ts.queue_position),
            .save_path      = ts.save_path,
            .size           = size,
            .state          = ts.state,
            .total          = ts.total,
            .total_done     = ts.total_done,
            .upload_rate    = ts.upload_rate,
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

    if (page_beg >= torrents.size())
    {
        return cb.Error(-2, "Invalid page - too large.");
    }

    cb.Ok(TorrentsListRes{
        .torrents = std::vector(
            torrents.begin() + page_beg,
            torrents.begin() + page_end),
        .torrents_total = static_cast<int>(torrents.size())
    });
}
