#include "torrentslist.hpp"

#include <boost/log/trivial.hpp>

#include "../../query/pql.hpp"
#include "../../sessions.hpp"
#include "../../torrentclientdata.hpp"
#include "../../utils/eta.hpp"
#include "../../utils/ratio.hpp"

using porla::Methods::TorrentsList;
using porla::Methods::TorrentsListFilters;
using porla::Methods::TorrentsListReq;
using porla::Methods::TorrentsListRes;

static const std::map<std::pair<std::string, bool>, std::function<bool(const lt::torrent_status&, const lt::torrent_status&)>> TorrentSort =
{
    {{"download_rate", false},  [](auto const& lhs, auto const& rhs) { return lhs.download_rate > rhs.download_rate; }},
    {{"download_rate", true},   [](auto const& lhs, auto const& rhs) { return lhs.download_rate < rhs.download_rate; }},
    {
        {"eta", false},
        [](auto const& lhs, auto const& rhs)
        {
            const auto lhs_eta = porla::Utils::Ratio(lhs);
            const auto rhs_eta = porla::Utils::Ratio(rhs);
            if (lhs_eta < 0) return false;
            if (rhs_eta < 0) return true;
            return lhs_eta > rhs_eta;
        }
    },
    {
        {"eta", true},
        [](auto const& lhs, auto const& rhs)
        {
            const auto lhs_eta = porla::Utils::Ratio(lhs);
            const auto rhs_eta = porla::Utils::Ratio(rhs);
            if (lhs_eta < 0) return false;
            if (rhs_eta < 0) return true;
            return lhs_eta < rhs_eta;
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
    {{"ratio", false},          [](auto const& lhs, auto const& rhs) { return porla::Utils::Ratio(lhs) > porla::Utils::Ratio(rhs); }},
    {{"ratio", true},           [](auto const& lhs, auto const& rhs) { return porla::Utils::Ratio(lhs) < porla::Utils::Ratio(rhs); }},
    {{"save_path", false},      [](auto const& lhs, auto const& rhs) { return strcmp(lhs.save_path.c_str(), rhs.save_path.c_str()) > 0; }},
    {{"save_path", true},       [](auto const& lhs, auto const& rhs) { return strcmp(lhs.save_path.c_str(), rhs.save_path.c_str()) < 0; }},
    {{"total", false},          [](auto const& lhs, auto const& rhs) { return lhs.total > rhs.total; }},
    {{"total", true},           [](auto const& lhs, auto const& rhs) { return lhs.total < rhs.total; }},
    {{"total_done", false},     [](auto const& lhs, auto const& rhs) { return lhs.total_done > rhs.total_done; }},
    {{"total_done", true},      [](auto const& lhs, auto const& rhs) { return lhs.total_done < rhs.total_done; }},
    {{"upload_rate", false},    [](auto const& lhs, auto const& rhs) { return lhs.upload_rate > rhs.upload_rate; }},
    {{"upload_rate", true},     [](auto const& lhs, auto const& rhs) { return lhs.upload_rate < rhs.upload_rate; }},
};

static const auto MapTorrentItem = [](
    const TorrentsListFilters& filters,
    const std::shared_ptr<porla::Sessions::SessionState>& state,
    const std::optional<std::function<bool(const lt::torrent_status&)>>& filter_query,
    const std::tuple<lt::torrent_handle, lt::torrent_status>& pair,
    std::vector<lt::torrent_status>& t)
{
    const auto& [ handle, ts ] = pair;

    if (!handle.is_valid())
    {
        return;
    }

    const auto client_data = handle.userdata().get<porla::TorrentClientData>();

    if (filter_query.has_value() && !filter_query.value()(ts))
    {
        return;
    }

    if (filters.category.has_value() && filters.category.value() != client_data->category)
    {
        return;
    }

    if (filters.errc.has_value())
    {
        if (filters.errc.value() == false && ts.errc.value() != 0) return;
        if (filters.errc.value() == true  && ts.errc.value() == 0) return;
    }

    if (filters.flags.has_value())
    {
        const auto has_all_flags = std::all_of(
            filters.flags->begin(),
            filters.flags->end(),
            [&ts](const auto& flag)
            {
                if (flag == "auto_managed")  return (ts.flags & lt::torrent_flags::auto_managed) != 0;
                if (flag == "!auto_managed") return (ts.flags & lt::torrent_flags::auto_managed) == 0;
                if (flag == "paused")        return (ts.flags & lt::torrent_flags::paused)       != 0;
                if (flag == "!paused")       return (ts.flags & lt::torrent_flags::paused)       == 0;
                return false;
            });
    }

    if (filters.save_path.has_value() && filters.save_path.value() != ts.save_path)
    {
        return;
    }

    if (filters.state.has_value())
    {
        const auto has_any_state = std::any_of(
            filters.state->begin(),
            filters.state->end(),
            [&ts](const auto& state)
            {
                if (state == "checking_files"       && ts.state == lt::torrent_status::checking_files)       return true;
                if (state == "checking_resume_data" && ts.state == lt::torrent_status::checking_resume_data) return true;
                if (state == "downloading_metadata" && ts.state == lt::torrent_status::downloading_metadata) return true;
                if (state == "downloading"          && ts.state == lt::torrent_status::downloading)          return true;
                if (state == "finished"             && ts.state == lt::torrent_status::finished)             return true;
                if (state == "seeding"              && ts.state == lt::torrent_status::seeding)              return true;
                return false;
            });

        if (!has_any_state)
        {
            return;
        }
    }

    if (filters.tags.has_value())
    {
        const auto has_all_tags = std::all_of(
            filters.tags->begin(),
            filters.tags->end(),
            [&client_data](const auto& tag) { return client_data->tags.contains(tag); });

        if (!has_all_tags)
        {
            return;
        }
    }

    t.emplace_back(ts);
};

TorrentsList::TorrentsList(porla::Sessions& sessions)
    : m_sessions(sessions)
{
}

void TorrentsList::Invoke(const TorrentsListReq& req, WriteCb<TorrentsListRes> cb)
{
    const std::string order_by     = req.order_by.value_or("queue_position");
    const bool        order_by_asc = req.order_by_dir.value_or("asc") == "asc";

    const auto& sorter = TorrentSort.find({ order_by, order_by_asc });

    if (sorter == TorrentSort.end())
    {
        return cb.Error(-1, "Invalid field in 'order_by'", {{"field", order_by }});
    }

    std::optional<std::function<bool(const lt::torrent_status&)>> filter_query;
    std::unique_ptr<Query::PQL::Filter> filter_ptr;

    std::optional<int> filter_session_id;

    if (req.filters.has_value()
        && req.filters->query.has_value())
    {
        try
        {
            filter_ptr   = Query::PQL::Parse(req.filters->query.value());
            filter_query = [&filter_ptr](const lt::torrent_status& ts) { return filter_ptr->Includes(ts); };
        }
        catch (const Query::QueryError& qe)
        {
            return cb.Error(-1000, qe.what(), {{"pos", qe.pos()}});
        }
    }

    if (req.filters.has_value()
        && req.filters->session_id.has_value())
    {
        filter_session_id = req.filters->session_id.value();
    }

    const int global_total_torrents = std::accumulate(
        m_sessions.All().begin(),
        m_sessions.All().end(),
        0,
        [](int current, const auto& state)
        {
            return current + state.second->torrents.size();
        });

    const auto& session_state = req.filters.has_value()
        ? req.filters->session_id.has_value()
            ? m_sessions.Get(req.filters->session_id.value())
            : m_sessions.Default()
        : m_sessions.Default();

    if (session_state == nullptr)
    {
        return cb.Error(-1, "Session not found");
    }

    std::vector<lt::torrent_status> torrents;
    torrents.reserve(session_state->torrents.size());

    for (const auto& [_, pair] : session_state->torrents)
    {
        MapTorrentItem(
            req.filters.value_or(TorrentsListFilters{}),
            session_state,
            filter_query,
            pair,
            torrents);
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
