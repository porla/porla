#include "torrents.hpp"

#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/log/trivial.hpp>
#include <boost/signals2.hpp>
#include <libtorrent/load_torrent.hpp>
#include <libtorrent/magnet_uri.hpp>
#include <sodium.h>

#include "resolve.hpp"

#include "../functionmethod.hpp"
#include "../jsonrpc.hpp"

#include "../../events.hpp"
#include "../../json/lttypes.hpp"
#include "../../json/utils.hpp"
#include "../../query/pql.hpp"
#include "../../sessions/torrent.hpp"
#include "../../sessions/session.hpp"
#include "../../sessions/sessions.hpp"
#include "../../sessions/torrentevent.hpp"
#include "../../torrentclientdata.hpp"
#include "../../torrents.hpp"
#include "../../utils/eta.hpp"
#include "../../utils/limits.hpp"
#include "../../utils/ratio.hpp"

using boost::asio::awaitable;

namespace porla::Rpc::Methods::Torrents
{
    namespace
    {
        struct TorrentRef
        {
            lt::info_hash_t    info_hash;
            std::optional<int> session_id;
        };

        struct AddReq
        {
            std::optional<std::string>                           category;
            std::optional<int>                                   download_limit;
            std::optional<lt::torrent_flags_t>                   flags;
            std::optional<std::string>                           magnet_uri;
            std::optional<int>                                   max_connections;
            std::optional<int>                                   max_uploads;
            std::optional<std::map<std::string, nlohmann::json>> metadata;
            std::optional<std::string>                           name;
            std::optional<std::string>                           preset;
            std::optional<int>                                   preset_id;
            std::optional<std::string>                           save_path;
            std::optional<int>                                   session_id;
            std::optional<std::unordered_set<std::string>>       tags;
            std::optional<std::string>                           ti;
            std::optional<std::vector<std::string>>              trackers;
            std::optional<int>                                   upload_limit;
            std::optional<std::vector<std::string>>              url_seeds;
        };

        struct CountReq
        {
            std::optional<int> session_id;
        };

        struct CountRes
        {
            std::map<std::string, int> categories;
            int                        downloading;
            int                        downloading_queued;
            int                        error;
            int                        finished;
            int                        paused;
            int                        seeding;
            int                        seeding_queued;
            int                        total;
            std::map<std::string, int> tags;
            std::map<std::string, int> trackers;
        };

        struct FilesListRes
        {
            libtorrent::file_storage  file_storage;
            libtorrent::renamed_files renamed_files;
        };

        struct FilesPrioritizeReq
        {
            struct FilePrio
            {
                libtorrent::file_index_t        index;
                libtorrent::download_priority_t priority;
            };

            libtorrent::info_hash_t info_hash;
            std::optional<int>      session_id;
            std::vector<FilePrio>   priorities;
        };


        struct FilesRenameReq
        {
            libtorrent::info_hash_t info_hash;
            std::optional<int>      session_id;
            int                     file_index;
            std::string             file_path;
        };

        struct ListFilters
        {
            std::optional<std::string>                     category;
            std::optional<bool>                            errc;
            std::optional<std::unordered_set<std::string>> flags;
            std::optional<std::string>                     query;
            std::optional<int>                             session_id;
            std::optional<std::string>                     save_path;
            std::optional<std::unordered_set<std::string>> state;
            std::optional<std::unordered_set<std::string>> status;
            std::optional<std::unordered_set<std::string>> tags;
        };

        struct ListReq
        {
            std::optional<ListFilters> filters;
            std::optional<int> page;
            std::optional<int> page_size;
            std::optional<std::string> order_by;
            std::optional<std::string> order_by_dir;
        };

        struct ListRes
        {
            std::string                             order_by;
            std::string                             order_by_dir;
            int                                     page;
            int                                     page_size;
            std::vector<libtorrent::torrent_status> torrents;
            int                                     torrents_total;
            int                                     torrents_total_unfiltered;
        };

        struct MigrateReq
        {
            libtorrent::info_hash_t info_hash;
            std::optional<int>      session_id;
            int                     target_session_id;
        };

        struct MigrateRes
        {
            libtorrent::info_hash_t info_hash;
            int                     session_id;
        };

        struct MoveReq
        {
            std::optional<std::string> flags;
            libtorrent::info_hash_t    info_hash;
            std::string                path;
            std::optional<int>         session_id;
        };

        struct PeersAddReq
        {
            libtorrent::info_hash_t                        info_hash;
            std::optional<int>                             session_id;
            std::vector<std::tuple<std::string, uint16_t>> peers;
        };

        struct PropertiesSetReq
        {
            libtorrent::info_hash_t info_hash;
            std::optional<int>      session_id;

            std::optional<std::string>                     category;
            std::optional<int>                             download_limit;
            std::optional<libtorrent::torrent_flags_t>     flags;
            std::optional<libtorrent::torrent_flags_t>     flags_mask;
            std::optional<int>                             max_connections;
            std::optional<int>                             max_uploads;
            std::optional<std::unordered_set<std::string>> tags;
            std::optional<int>                             upload_limit;
        };

        struct QueueSetReq
        {
            libtorrent::info_hash_t      info_hash;
            std::optional<int>           session_id;
            libtorrent::queue_position_t queue_position;
        };

        struct RemoveReq
        {
            libtorrent::info_hash_t              info_hash;
            std::optional<int>                   session_id;
            std::optional<bool>                  remove_data;
        };

        struct TrackersAddReq
        {
            libtorrent::info_hash_t info_hash;
            std::optional<int>      session_id;
            std::string             url;
            std::optional<uint8_t>  tier;
        };

        PORLA_JSON_REQUEST(TorrentRef, info_hash, session_id)

        PORLA_JSON_REQUEST(
            AddReq,
            category,
            download_limit,
            flags,
            magnet_uri,
            max_connections,
            max_uploads,
            metadata,
            name,
            preset,
            preset_id,
            save_path,
            session_id,
            tags,
            ti,
            trackers,
            upload_limit,
            url_seeds)

        PORLA_JSON_REQUEST(
            CountReq,
            session_id)

        PORLA_JSON_REQUEST(
            FilesPrioritizeReq::FilePrio,
            index,
            priority)

        PORLA_JSON_REQUEST(
            FilesPrioritizeReq,
            info_hash,
            session_id,
            priorities)

        PORLA_JSON_REQUEST(
            FilesRenameReq,
            info_hash,
            session_id,
            file_index,
            file_path)

        PORLA_JSON_REQUEST(
            ListFilters,
            category,
            errc,
            flags,
            query,
            session_id,
            save_path,
            state,
            status,
            tags)

        PORLA_JSON_REQUEST(
            ListReq,
            filters,
            page,
            page_size,
            order_by,
            order_by_dir)

        PORLA_JSON_REQUEST(
            MigrateReq,
            info_hash,
            session_id,
            target_session_id)

        PORLA_JSON_REQUEST(
            MoveReq,
            flags,
            info_hash,
            path,
            session_id)

        PORLA_JSON_REQUEST(
            PeersAddReq,
            info_hash,
            session_id,
            peers)

        PORLA_JSON_REQUEST(
            PropertiesSetReq,
            info_hash,
            session_id,
            category,
            download_limit,
            flags,
            flags_mask,
            max_connections,
            max_uploads,
            tags,
            upload_limit)

        PORLA_JSON_REQUEST(
            QueueSetReq,
            info_hash,
            session_id,
            queue_position)

        PORLA_JSON_REQUEST(
            RemoveReq,
            info_hash,
            session_id,
            remove_data)

        PORLA_JSON_REQUEST(
            TrackersAddReq,
            info_hash,
            session_id,
            url,
            tier)

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
            CountRes,
            categories,
            downloading,
            downloading_queued,
            error,
            finished,
            paused,
            seeding,
            seeding_queued,
            total,
            tags,
            trackers)

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
            ListRes,
            order_by,
            order_by_dir,
            page,
            page_size,
            torrents,
            torrents_total,
            torrents_total_unfiltered)

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
            MigrateRes,
            info_hash,
            session_id)

        void to_json(nlohmann::json& json, const FilesListRes& res)
        {
            nlohmann::json files = nlohmann::json::array();

            const auto& storage = res.file_storage;

            lt::filenames fn(res.file_storage, res.renamed_files);

            for (const auto idx : fn.file_range())
            {
                files.push_back({
                    {"absolute_path", fn.file_absolute_path(idx)},
                    {"flags",         fn.file_flags(idx)},
                    {"index",         static_cast<int>(idx)},
                    {"offset",        fn.file_offset(idx)},
                    {"path",          fn.file_path(idx)},
                    {"size",          fn.file_size(idx)},
                    {"symlink",       fn.symlink(idx)}
                });
            }

            json = {
                {"files", files},
            };
        }

        static const std::map<std::pair<std::string, bool>, std::function<bool(const lt::torrent_status&, const lt::torrent_status&)>> TorrentSort =
        {
            {{"download_rate", false},  [](auto const& lhs, auto const& rhs) { return lhs.download_rate > rhs.download_rate; }},
            {{"download_rate", true},   [](auto const& lhs, auto const& rhs) { return lhs.download_rate < rhs.download_rate; }},
            {
                {"eta", false},
                [](auto const& lhs, auto const& rhs)
                {
                    const auto lhs_eta = porla::Utils::ETA(lhs).count();
                    const auto rhs_eta = porla::Utils::ETA(rhs).count();
                    if (lhs_eta < 0) return false;
                    if (rhs_eta < 0) return true;
                    return lhs_eta > rhs_eta;
                }
            },
            {
                {"eta", true},
                [](auto const& lhs, auto const& rhs)
                {
                    const auto lhs_eta = porla::Utils::ETA(lhs).count();
                    const auto rhs_eta = porla::Utils::ETA(rhs).count();
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
                    if (static_cast<int>(lhs.queue_position) < 0) return false;
                    if (static_cast<int>(rhs.queue_position) < 0) return true;
                    return lhs.queue_position > rhs.queue_position;
                }
            },
            {
                {"queue_position", true},
                [](auto const& lhs, auto const& rhs)
                {
                    if (static_cast<int>(lhs.queue_position) < 0) return false;
                    if (static_cast<int>(rhs.queue_position) < 0) return true;
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

        void ThrowError(const porla::TorrentsAddResult& result)
        {
            using AddError = porla::TorrentsAddResult::Error;

            switch(result.error)
            {
            case AddError::SessionNotFound:  throw Error(-1, result.what, {{ "session_id", result.session_id }});
            case AddError::SessionNotLoaded: throw Error(-2, result.what, {{ "session_id", result.session_id }});
            case AddError::MissingInfoHash:  throw Error(-4, result.what);
            case AddError::AlreadyInSession: throw Error(-5, result.what);
            case AddError::MissingSavePath:  throw Error(-6, result.what);
            case AddError::Failed:           throw Error(-7, "Failed to add torrent to session", {{ "what", result.what }});
            case AddError::InvalidData:      throw Error(-8, result.what);
            case AddError::MissingPreset:    throw Error(-9, result.what);
            case AddError::None:             return;
            }
        }

        awaitable<nlohmann::json> Add(porla::Torrents& torrents, const AddReq& req)
        {
            lt::error_code ec;
            lt::add_torrent_params p;
            
            if (req.ti.has_value())
            {
                const auto data = req.ti.value();

                std::string output;
                output.resize(data.size() / 4 * 3 + 3);

                std::size_t bin_len = 0;

                const int result = sodium_base642bin(
                    reinterpret_cast<unsigned char*>(output.data()),
                    output.size(),
                    data.data(),
                    data.size(),
                    nullptr,
                    &bin_len,
                    nullptr,
                    sodium_base64_VARIANT_ORIGINAL);

                if (result != 0)
                {
                    throw Error(-2, "Failed to parse 'ti'");
                }

                output.resize(bin_len);

                p = lt::load_torrent_buffer(output, ec, {});

                if (ec)
                {
                    throw Error(-2, "Failed to parse 'ti'");
                }
            }
            else if (req.magnet_uri.has_value())
            {
                p = lt::parse_magnet_uri(req.magnet_uri.value(), ec);

                if (ec)
                {
                    throw Error(-2, "Failed to parse 'magnet_uri'");
                }
            }
            else
            {
                throw Error(-3, "Either 'ti' or 'magnet_uri' must be set");
            }

            const auto prepared = torrents.Prepare(p, porla::TorrentsAddOptions{
                .session_id = req.session_id,
                .preset_id  = req.preset_id,
                .preset     = req.preset
            });

            if (!prepared)
            {
                ThrowError(prepared);
            }

            // explicit values
            if (req.download_limit.has_value())  p.download_limit  = req.download_limit.value();
            if (req.flags.has_value())           p.flags           = req.flags.value();
            if (req.max_connections.has_value()) p.max_connections = req.max_connections.value();
            if (req.max_uploads.has_value())     p.max_uploads     = req.max_uploads.value();
            if (req.name.has_value())            p.name            = req.name.value();
            if (req.save_path.has_value()
                && req.save_path->length() > 0)  p.save_path       = req.save_path.value();
            if (req.trackers.has_value())        p.trackers        = req.trackers.value();
            if (req.upload_limit.has_value())    p.upload_limit    = req.upload_limit.value();
            if (req.url_seeds.has_value())       p.url_seeds       = req.url_seeds.value();

            // userdata values
            auto* client_data = p.userdata.get<TorrentClientData>();

            if (req.category.has_value()) client_data->category = req.category.value();
            if (req.metadata.has_value()) client_data->metadata = req.metadata.value();
            if (req.tags.has_value())     client_data->tags     = req.tags.value();

            const auto added = torrents.Add(std::move(p));

            if (!added)
            {
                ThrowError(added);
            }

            co_return nlohmann::json({
                {"info_hash", added.info_hash},
                {"session_id", added.session_id}
            });
        }

        awaitable<CountRes> Count(porla::Sessions& sessions, const CountReq& req)
        {
            auto resolved_session = ResolveSession(sessions, req.session_id);

            CountRes res{};
            res.total = resolved_session.session->Count();

            for (const auto& [ _, t ] : resolved_session.session->Torrents())
            {
                const auto& ts = t.status;

                if ((ts.state == lt::torrent_status::state_t::downloading
                    || ts.state == lt::torrent_status::state_t::downloading_metadata)
                    && !(ts.flags & lt::torrent_flags::paused))
                {
                    res.downloading++;
                }

                if ((ts.state == lt::torrent_status::state_t::downloading
                    || ts.state == lt::torrent_status::state_t::downloading_metadata)
                    && (ts.flags & lt::torrent_flags::auto_managed)
                    && (ts.flags & lt::torrent_flags::paused))
                {
                    res.downloading_queued++;
                }

                if (ts.errc)
                {
                    res.error++;
                }

                if ((ts.state == lt::torrent_status::state_t::finished
                    || ts.state == lt::torrent_status::state_t::seeding)
                    && !(ts.flags & lt::torrent_flags::auto_managed)
                    && (ts.flags & lt::torrent_flags::paused))
                {
                    res.finished++;
                }

                if ((ts.state == lt::torrent_status::state_t::downloading
                    || ts.state == lt::torrent_status::state_t::downloading_metadata)
                    && !(ts.flags & lt::torrent_flags::auto_managed)
                    && (ts.flags & lt::torrent_flags::paused))
                {
                    res.paused++;
                }

                if ((ts.state == lt::torrent_status::state_t::finished
                    || ts.state == lt::torrent_status::state_t::seeding)
                    && !(ts.flags & lt::torrent_flags::paused))
                {
                    res.seeding++;
                }

                if ((ts.state == lt::torrent_status::state_t::finished
                    || ts.state == lt::torrent_status::state_t::seeding)
                    && (ts.flags & lt::torrent_flags::auto_managed)
                    && (ts.flags & lt::torrent_flags::paused))
                {
                    res.seeding_queued++;
                }

                res.trackers[ts.current_tracker]++;

                if (t.data)
                {
                    if (t.data->category.has_value() && !t.data->category->empty())
                    {
                        res.categories[t.data->category.value()]++;
                    }

                    for (const auto& tag : t.data->tags)
                    {
                        if (tag.empty()) continue;
                        res.tags[tag]++;
                    }
                }
            }

            co_return res;
        }

        awaitable<nlohmann::json> TrackersList(porla::Sessions& sessions, const TorrentRef& req)
        {
            const auto resolved = ResolveTorrent(sessions, req.session_id, req.info_hash);
            co_return nlohmann::json({
                {"trackers", resolved.torrent->status.handle.trackers()}
            });
        }

        awaitable<FilesListRes> FilesList(porla::Sessions& sessions, const TorrentRef& req)
        {
            const auto resolved = ResolveTorrent(
                sessions,
                req.session_id,
                req.info_hash);

            if (auto tf = resolved.torrent->status.torrent_file.lock())
            {
                co_return FilesListRes{
                    .file_storage  = tf->layout(),
                    .renamed_files = resolved.torrent->status.handle.get_renamed_files()
                };
            }

            throw Error(-4, "Failed to lock torrent file");
        }

        awaitable<nlohmann::json> FilesPriorities(porla::Sessions& sessions, const TorrentRef& req)
        {
            const auto resolved = ResolveTorrent(sessions, req.session_id, req.info_hash);

            co_return nlohmann::json({
                {"priorities", resolved.torrent->status.handle.get_file_priorities()}
            });
        }

        awaitable<void> FilesPrioritize(porla::Sessions& sessions, const FilesPrioritizeReq& req)
        {
            const auto resolved = ResolveTorrent(sessions, req.session_id, req.info_hash);

            const auto tf = resolved.torrent->status.torrent_file.lock();

            if (!tf)
            {
                co_return;
            }

            const auto& th        = resolved.torrent->status.handle;
            const auto  num_files = tf->num_files();

            // set each priority on its own - libtorrent merges these while a disk job is running.
            // a get_file_priorities/prioritize_files round trip would block the io thread and
            // could overwrite priorities that libtorrent has queued but not applied yet.
            for (const auto& fp : req.priorities)
            {
                if (fp.index < lt::file_index_t{0} || static_cast<int>(fp.index) >= num_files)
                {
                    continue;
                }

                th.file_priority(fp.index, fp.priority);
            }

            co_return;
        }

        awaitable<nlohmann::json> FilesProgress(porla::Sessions& sessions, const TorrentRef& req)
        {
            const auto resolved = ResolveTorrent(sessions, req.session_id, req.info_hash);

            co_return nlohmann::json({
                {"progress", resolved.torrent->status.handle.file_progress()}
            });
        }

        awaitable<void> FilesRename(porla::Sessions& sessions, const FilesRenameReq& req)
        {
            const auto resolved = ResolveTorrent(sessions, req.session_id, req.info_hash);

            resolved.torrent->status.handle.rename_file(
                lt::file_index_t{req.file_index},
                req.file_path);

            co_return;
        }

        awaitable<nlohmann::json> Get(porla::Sessions& sessions, const TorrentRef& req)
        {
            const auto resolved = ResolveTorrent(sessions, req.session_id, req.info_hash);

            co_return nlohmann::json({
                {"torrent", resolved.torrent->status}
            });
        }

        awaitable<ListRes> List(porla::Sessions& sessions, const ListReq& req)
        {
            const int         page         = req.page.value_or(0);
            const int         page_size    = req.page_size.value_or(50);
            const std::string order_by     = req.order_by.value_or("queue_position");
            const bool        order_by_asc = req.order_by_dir.value_or("asc") == "asc";

            if (page < 0)
            {
                throw Error(-1, "'page' must be >= 0");
            }

            if (page_size < 1 || page_size > 1000)
            {
                throw Error(-1, "'page_size' must be between 1 and 1000");
            }

            const auto& sorter = TorrentSort.find({ order_by, order_by_asc });

            if (sorter == TorrentSort.end())
            {
                throw Error(-2, "Invalid field in 'order_by'", {{"field", order_by }});
            }

            std::optional<Query::Filter> filter_query;

            if (req.filters.has_value()
                && req.filters->query.has_value())
            {
                try
                {
                    filter_query = Query::PQL::Parse(req.filters->query.value());
                }
                catch (const Query::QueryError& qe)
                {
                    throw Error(-1000, qe.what(), {{"start", qe.start()}, {"end", qe.end()}});
                }
            }

            const auto session_state = ResolveSession(
                sessions,
                req.filters.has_value() ? req.filters->session_id : std::nullopt);

            std::vector<const lt::torrent_status*> torrents;
            torrents.reserve(session_state.session->Count());

            const auto now = std::time(nullptr);

            for (const auto& [_, t] : session_state.session->Torrents())
            {
                const auto& ts     = t.status;
                const auto& handle = ts.handle;

                if (!handle.is_valid())
                {
                    continue;
                }

                // If we have no filters, just add
                if (!req.filters.has_value())
                {
                    torrents.push_back(&ts);
                    continue;
                }

                const auto client_data = handle.userdata().get<porla::TorrentClientData>();

                if (filter_query.has_value())
                {
                    const Fields::Context ctx{
                        .status      = ts,
                        .client_data = client_data,
                        .now         = now
                    };

                    if (!filter_query.value()(ctx))
                    {
                        continue;
                    }
                }

                if (client_data && req.filters->category.has_value() && req.filters->category.value() != client_data->category)
                {
                    continue;
                }


                if (req.filters->save_path.has_value() && req.filters->save_path.value() != ts.save_path)
                {
                    continue;
                }

                if (req.filters->state.has_value())
                {
                    const auto has_any_state = std::any_of(
                        req.filters->state->begin(),
                        req.filters->state->end(),
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
                        continue;
                    }
                }

                if (req.filters->status.has_value())
                {
                    const auto has_any_status = std::any_of(
                        req.filters->status->begin(),
                        req.filters->status->end(),
                        [&ts](const auto& status)
                        {
                            if (status == "downloading"
                                && (ts.state == lt::torrent_status::downloading || ts.state == lt::torrent_status::downloading_metadata)
                                && !(ts.flags & lt::torrent_flags::paused))
                            {
                                return true;
                            }

                            if (status == "downloading_queued"
                                && (ts.state == lt::torrent_status::downloading || ts.state == lt::torrent_status::downloading_metadata)
                                && (ts.flags & lt::torrent_flags::auto_managed)
                                && (ts.flags & lt::torrent_flags::paused))
                            {
                                return true;
                            }

                            if (status == "error"
                                && ts.errc)
                            {
                                return true;
                            }

                            if (status == "finished"
                                && (ts.state == lt::torrent_status::finished || ts.state == lt::torrent_status::seeding)
                                && !(ts.flags & lt::torrent_flags::auto_managed)
                                && (ts.flags & lt::torrent_flags::paused))
                            {
                                return true;
                            }

                            if (status == "paused"
                                && (ts.state == lt::torrent_status::downloading || ts.state == lt::torrent_status::downloading_metadata)
                                && !(ts.flags & lt::torrent_flags::auto_managed)
                                && (ts.flags & lt::torrent_flags::paused))
                            {
                                return true;
                            }

                            if (status == "seeding"
                                && (ts.state == lt::torrent_status::finished || ts.state == lt::torrent_status::seeding)
                                && !(ts.flags & lt::torrent_flags::paused))
                            {
                                return true;
                            }

                            if (status == "seeding_queued"
                                && (ts.state == lt::torrent_status::finished || ts.state == lt::torrent_status::seeding)
                                && (ts.flags & lt::torrent_flags::auto_managed)
                                && (ts.flags & lt::torrent_flags::paused))
                            {
                                return true;
                            }

                            return false;
                        });

                    if (!has_any_status)
                    {
                        continue;
                    }
                }

                if (client_data && req.filters->tags.has_value())
                {
                    const auto has_all_tags = std::all_of(
                        req.filters->tags->begin(),
                        req.filters->tags->end(),
                        [&client_data](const auto& tag) { return client_data->tags.contains(tag); });

                    if (!has_all_tags)
                    {
                        continue;
                    }
                }

                torrents.push_back(&ts);
            }

            const auto total    = static_cast<std::int64_t>(torrents.size());
            const auto page_beg = std::min<std::int64_t>(
                static_cast<std::int64_t>(page) * page_size, total);
            const auto page_end = std::min<std::int64_t>(page_beg + page_size, total);

            const auto cmp = [&sorter](const lt::torrent_status* lhs, const lt::torrent_status* rhs)
            {
                if (sorter->second(*lhs, *rhs)) return true;
                if (sorter->second(*rhs, *lhs)) return false;
                return lhs->info_hashes < rhs->info_hashes;
            };

            if (page_end >= total / 2)
            {
                std::sort(torrents.begin(), torrents.end(), cmp);
            }
            else
            {
                std::partial_sort(
                    torrents.begin(),
                    torrents.begin() + page_end,
                    torrents.end(),
                    cmp);
            }

            std::vector<lt::torrent_status> page_items;
            page_items.reserve(static_cast<std::size_t>(page_end - page_beg));

            for (auto i = page_beg; i < page_end; i++)
            {
                page_items.push_back(*torrents[i]);
            }

            co_return ListRes{
                .order_by                  = req.order_by.value_or("queue_position"),
                .order_by_dir              = req.order_by_dir.value_or("asc"),
                .page                      = req.page.value_or(0),
                .page_size                 = req.page_size.value_or(50),
                .torrents                  = std::move(page_items),
                .torrents_total            = static_cast<int>(torrents.size()),
                .torrents_total_unfiltered = static_cast<int>(session_state.session->Count())
            };
        }

        // Removes the torrent from its session and, once torrent.removed arrives, adds it to the
        // target. If the target is gone or refuses it, the torrent is added back to the source.
        awaitable<MigrateRes> Migrate(porla::Events& events, porla::Sessions& sessions, const MigrateReq& req)
        {
            auto resolved = ResolveTorrent(sessions, req.session_id, req.info_hash);

            if (req.target_session_id == resolved.session->Id())
            {
                throw Error(-32602, "target_session_id must differ from session_id", {{"field", "target_session_id"}});
            }

            auto target = sessions.Get(req.target_session_id);

            if (!target)
            {
                throw Error(-2, "Target session not loaded");
            }

            if (target->Contains(resolved.torrent->status.info_hashes))
            {
                throw Error(-5, "Torrent already in target session");
            }

            const auto handle = resolved.torrent->status.handle;
            auto       params = handle.get_resume_data();

            TorrentClientData client_data;

            if (const auto prev = handle.userdata().get<TorrentClientData>())
            {
                client_data.category     = prev->category;
                client_data.completed_at = prev->completed_at;
                client_data.metadata     = prev->metadata;
                client_data.tags         = prev->tags;
            }

            // Hold only weak references while waiting, so either session can still be unloaded.
            const int                     source_id      = resolved.session->Id();
            std::weak_ptr<porla::Session> source_session = resolved.session;
            std::weak_ptr<porla::Session> target_session = target;

            target.reset();

            // torrent.removed cancels the timer, so the wait ends on removal or after 30 seconds.
            boost::asio::steady_timer timeout(co_await boost::asio::this_coro::executor, std::chrono::seconds(30));
            bool                      removed = false;

            boost::signals2::scoped_connection connection = events.On("torrent.removed",
                [&](const porla::Event& event)
                {
                    const auto& ev = static_cast<const porla::TorrentEvent&>(event);

                    if (ev.session_id == source_id && ev.info_hash == params.info_hashes)
                    {
                        removed = true;
                        timeout.cancel();
                    }
                });

            resolved.session->Libtorrent().remove_torrent(handle);
            resolved = {};

            co_await timeout.async_wait(boost::asio::as_tuple(boost::asio::use_awaitable));
            connection.disconnect();

            if (!removed)
            {
                BOOST_LOG_TRIVIAL(error) << "Timed out waiting for removal of " << params.info_hashes << " at " << params.save_path;

                throw Error(-32603, "Timed out waiting for removal");
            }

            // Best case, target session exists and accepts the torrent
            if (auto session = target_session.lock())
            {
                params.userdata = lt::client_data_t(new TorrentClientData(client_data));

                if (session->Add(params))
                {
                    BOOST_LOG_TRIVIAL(info) << "Torrent " << params.info_hashes << " migrated to session " << session->Name();

                    co_return MigrateRes{
                        .info_hash  = params.info_hashes,
                        .session_id = session->Id()
                    };
                }
            }

            // Target session did not exist, or it did not accept the torrent
            if (auto session = source_session.lock())
            {
                params.userdata = lt::client_data_t(new TorrentClientData(client_data));

                if (session->Add(params))
                {
                    BOOST_LOG_TRIVIAL(warning) << "Target session did not exist or did not accept torrent - added back to source";

                    throw Error(-6, "Torrent migration failed - added back to source session");
                }
            }

            // We now have a failure on our hand, a torrent is missing.
            BOOST_LOG_TRIVIAL(error)
                << "Failed to migrate torrent and source did not accept it back - lost torrent " << params.info_hashes << " at " << params.save_path;

            throw Error(-7, "Migration failed - neither target nor source accepted the torrent");
        }

        awaitable<void> Move(porla::Sessions& sessions, const MoveReq& req)
        {
            const auto resolved = ResolveTorrent(sessions, req.session_id, req.info_hash);

            lt::move_flags_t flags = lt::move_flags_t::dont_replace;

            if (req.flags.has_value())
            {
                if (req.flags.value() == "always_replace_files") flags = lt::move_flags_t::always_replace_files;
                if (req.flags.value() == "dont_replace")         flags = lt::move_flags_t::dont_replace;
                if (req.flags.value() == "fail_if_exist")        flags = lt::move_flags_t::fail_if_exist;
            }

            if (!resolved.torrent->status.handle.is_valid())
            {
                throw Error(-4, "Torrent not valid");
            }

            resolved.torrent->status.handle.move_storage(req.path, flags);

            co_return;
        }

        awaitable<void> Pause(porla::Sessions& sessions, const TorrentRef& req)
        {
            const auto resolved = ResolveTorrent(sessions, req.session_id, req.info_hash);
            resolved.torrent->status.handle.pause();
            co_return;
        }

        awaitable<void> PeersAdd(porla::Sessions& sessions, const PeersAddReq& req)
        {
            const auto resolved = ResolveTorrent(sessions, req.session_id, req.info_hash);

            for (const auto& [ip, port] : req.peers)
            {
                boost::system::error_code ec;
                auto addr = boost::asio::ip::make_address(ip, ec);

                if (ec)
                {
                    BOOST_LOG_TRIVIAL(error) << "Failed to parse '" << ip << "': " << ec.message();
                    continue;
                }

                resolved.torrent->status.handle.connect_peer(boost::asio::ip::tcp::endpoint{addr,port});
            }

            co_return;
        }

        awaitable<nlohmann::json> PeersList(porla::Sessions& sessions, const TorrentRef& req)
        {
            const auto resolved = ResolveTorrent(sessions, req.session_id, req.info_hash);

            std::vector<lt::peer_info> peers;
            resolved.torrent->status.handle.get_peer_info(peers);

            co_return nlohmann::json({
                {"peers", peers}
            });
        }

        awaitable<nlohmann::json> PiecesGet(porla::Sessions& sessions, const TorrentRef& req)
        {
            const auto resolved = ResolveTorrent(sessions, req.session_id, req.info_hash);

            co_return nlohmann::json({
                {"pieces", resolved.torrent->status.pieces},
                {"verified_pieces", resolved.torrent->status.verified_pieces}
            });
        }

        awaitable<nlohmann::json> PropertiesGet(porla::Sessions& sessions, const TorrentRef& req)
        {
            const auto resolved = ResolveTorrent(sessions, req.session_id, req.info_hash);

            co_return nlohmann::json({
                {"download_limit", resolved.torrent->status.handle.download_limit()},
                {"flags", resolved.torrent->status.handle.flags()},
                {"max_connections", resolved.torrent->status.handle.max_connections()},
                {"max_uploads", resolved.torrent->status.handle.max_uploads()},
                {"upload_limit", resolved.torrent->status.handle.upload_limit()}
            });
        }

        awaitable<void> PropertiesSet(porla::Sessions& sessions, porla::Torrents& torrents, const PropertiesSetReq& req)
        {
            const auto resolved = ResolveTorrent(sessions, req.session_id, req.info_hash);

            // check all limits
            const auto download_limit  = req.download_limit  ? Utils::CheckRateLimit(*req.download_limit)  : std::nullopt;
            const auto upload_limit    = req.upload_limit    ? Utils::CheckRateLimit(*req.upload_limit)    : std::nullopt;
            const auto max_connections = req.max_connections ? Utils::CheckPeerLimit(*req.max_connections) : std::nullopt;
            const auto max_uploads     = req.max_uploads     ? Utils::CheckPeerLimit(*req.max_uploads)     : std::nullopt;

            if (req.download_limit  && !download_limit)  throw Error(-5, "Invalid download_limit");
            if (req.upload_limit    && !upload_limit)    throw Error(-5, "Invalid upload_limit");
            if (req.max_connections && !max_connections) throw Error(-5, "Invalid max_connections");
            if (req.max_uploads     && !max_uploads)     throw Error(-5, "Invalid max_uploads");

            if (req.category.has_value() || req.tags.has_value())
            {
                const auto result = torrents.UpdateClientData(
                    resolved.torrent->status.handle,
                    [&req](TorrentClientData& client_data)
                    {
                        if (req.category.has_value()) client_data.category = req.category.value();
                        if (req.tags.has_value())     client_data.tags     = req.tags.value();
                    });

                if (result == porla::Torrents::UpdateClientDataResult::NotFound)
                {
                    throw Error(-6, "Torrent has no client data - cannot set category or tags");
                }

                if (result == porla::Torrents::UpdateClientDataResult::InvalidData)
                {
                    throw Error(-8, "Category, tags and metadata must be valid UTF-8");
                }
            }

            if (req.flags.has_value() && req.flags_mask.has_value())
            {
                const auto flags = req.flags.value();
                const auto mask  = req.flags_mask.value();

                resolved.torrent->status.handle.set_flags(flags, mask);
            }

            if (download_limit)  resolved.torrent->status.handle.set_download_limit(*download_limit);
            if (max_connections) resolved.torrent->status.handle.set_max_connections(*max_connections);
            if (max_uploads)     resolved.torrent->status.handle.set_max_uploads(*max_uploads);
            if (upload_limit)    resolved.torrent->status.handle.set_upload_limit(*upload_limit);

            co_return;
        }

        awaitable<void> QueueBottom(porla::Sessions& sessions, const TorrentRef& req)
        {
            const auto resolved = ResolveTorrent(sessions, req.session_id, req.info_hash);
            resolved.torrent->status.handle.queue_position_bottom();
            co_return;
        }

        awaitable<void> QueueDown(porla::Sessions& sessions, const TorrentRef& req)
        {
            const auto resolved = ResolveTorrent(sessions, req.session_id, req.info_hash);
            resolved.torrent->status.handle.queue_position_down();
            co_return;
        }

        awaitable<void> QueueSet(porla::Sessions& sessions, const QueueSetReq& req)
        {
            const auto resolved = ResolveTorrent(sessions, req.session_id, req.info_hash);
            resolved.torrent->status.handle.queue_position_set(req.queue_position);
            co_return;
        }

        awaitable<void> QueueTop(porla::Sessions& sessions, const TorrentRef& req)
        {
            const auto resolved = ResolveTorrent(sessions, req.session_id, req.info_hash);
            resolved.torrent->status.handle.queue_position_top();
            co_return;
        }

        awaitable<void> QueueUp(porla::Sessions& sessions, const TorrentRef& req)
        {
            const auto resolved = ResolveTorrent(sessions, req.session_id, req.info_hash);
            resolved.torrent->status.handle.queue_position_up();
            co_return;
        }

        awaitable<void> Recheck(porla::Sessions& sessions, const TorrentRef& req)
        {
            const auto resolved = ResolveTorrent(sessions, req.session_id, req.info_hash);
            resolved.session->Recheck(resolved.torrent->status.info_hashes);
            co_return;
        }

        awaitable<void> Remove(porla::Sessions& sessions, const RemoveReq& req)
        {
            const auto resolved = ResolveTorrent(sessions, req.session_id, req.info_hash);

            resolved.session->Libtorrent().remove_torrent(
                resolved.torrent->status.handle,
                req.remove_data.value_or(false)
                    ? lt::session::delete_files
                    : lt::remove_flags_t{});

            co_return;
        }

        awaitable<void> Resume(porla::Sessions& sessions, const TorrentRef& req)
        {
            const auto resolved = ResolveTorrent(sessions, req.session_id, req.info_hash);
            resolved.torrent->status.handle.resume();
            co_return;
        }

        awaitable<void> TrackersAdd(porla::Sessions& sessions, const TrackersAddReq& req)
        {
            const auto resolved = ResolveTorrent(sessions, req.session_id, req.info_hash);

            lt::announce_entry ae;
            ae.url = req.url;

            if (req.tier.has_value())
            {
                ae.tier = req.tier.value();
            }

            resolved.torrent->status.handle.add_tracker(ae);

            co_return;
        }
    }

    void Register(boost::asio::io_context& io, JsonRpc& jsonrpc, porla::Events& events, porla::Sessions& sessions, porla::Torrents& torrents)
    {
        using Kind = porla::Auth::Context::Kind;

        auto ex = io.get_executor();

        jsonrpc.Register("torrents.add",              { Kind::User, Kind::Machine }, MakeMethod<AddReq>(ex, std::bind_front(Add, std::ref(torrents))));
        jsonrpc.Register("torrents.count",            { Kind::User, Kind::Machine }, MakeMethod<CountReq>(ex, std::bind_front(Count, std::ref(sessions))));
        jsonrpc.Register("torrents.files.list",       { Kind::User, Kind::Machine }, MakeMethod<TorrentRef>(ex, std::bind_front(FilesList, std::ref(sessions))));
        jsonrpc.Register("torrents.files.priorities", { Kind::User, Kind::Machine }, MakeMethod<TorrentRef>(ex, std::bind_front(FilesPriorities, std::ref(sessions))));
        jsonrpc.Register("torrents.files.prioritize", { Kind::User, Kind::Machine }, MakeMethod<FilesPrioritizeReq>(ex, std::bind_front(FilesPrioritize, std::ref(sessions))));
        jsonrpc.Register("torrents.files.progress",   { Kind::User, Kind::Machine }, MakeMethod<TorrentRef>(ex, std::bind_front(FilesProgress, std::ref(sessions))));
        jsonrpc.Register("torrents.files.rename",     { Kind::User, Kind::Machine }, MakeMethod<FilesRenameReq>(ex, std::bind_front(FilesRename, std::ref(sessions))));
        jsonrpc.Register("torrents.get",              { Kind::User, Kind::Machine }, MakeMethod<TorrentRef>(ex, std::bind_front(Get, std::ref(sessions))));
        jsonrpc.Register("torrents.list",             { Kind::User, Kind::Machine }, MakeMethod<ListReq>(ex, std::bind_front(List, std::ref(sessions))));
        jsonrpc.Register("torrents.migrate",          { Kind::User, Kind::Machine }, MakeMethod<MigrateReq>(ex, std::bind_front(Migrate, std::ref(events), std::ref(sessions))));
        jsonrpc.Register("torrents.move",             { Kind::User, Kind::Machine }, MakeMethod<MoveReq>(ex, std::bind_front(Move, std::ref(sessions))));
        jsonrpc.Register("torrents.pause",            { Kind::User, Kind::Machine }, MakeMethod<TorrentRef>(ex, std::bind_front(Pause, std::ref(sessions))));
        jsonrpc.Register("torrents.peers.add",        { Kind::User, Kind::Machine }, MakeMethod<PeersAddReq>(ex, std::bind_front(PeersAdd, std::ref(sessions))));
        jsonrpc.Register("torrents.peers.list",       { Kind::User, Kind::Machine }, MakeMethod<TorrentRef>(ex, std::bind_front(PeersList, std::ref(sessions))));
        jsonrpc.Register("torrents.pieces.get",       { Kind::User, Kind::Machine }, MakeMethod<TorrentRef>(ex, std::bind_front(PiecesGet, std::ref(sessions))));
        jsonrpc.Register("torrents.properties.get",   { Kind::User, Kind::Machine }, MakeMethod<TorrentRef>(ex, std::bind_front(PropertiesGet, std::ref(sessions))));
        jsonrpc.Register("torrents.properties.set",   { Kind::User, Kind::Machine }, MakeMethod<PropertiesSetReq>(ex, std::bind_front(PropertiesSet, std::ref(sessions), std::ref(torrents))));
        jsonrpc.Register("torrents.queue.bottom",     { Kind::User, Kind::Machine }, MakeMethod<TorrentRef>(ex, std::bind_front(QueueBottom, std::ref(sessions))));
        jsonrpc.Register("torrents.queue.down",       { Kind::User, Kind::Machine }, MakeMethod<TorrentRef>(ex, std::bind_front(QueueDown, std::ref(sessions))));
        jsonrpc.Register("torrents.queue.set",        { Kind::User, Kind::Machine }, MakeMethod<QueueSetReq>(ex, std::bind_front(QueueSet, std::ref(sessions))));
        jsonrpc.Register("torrents.queue.top",        { Kind::User, Kind::Machine }, MakeMethod<TorrentRef>(ex, std::bind_front(QueueUp, std::ref(sessions))));
        jsonrpc.Register("torrents.queue.up",         { Kind::User, Kind::Machine }, MakeMethod<TorrentRef>(ex, std::bind_front(QueueTop, std::ref(sessions))));
        jsonrpc.Register("torrents.recheck",          { Kind::User, Kind::Machine }, MakeMethod<TorrentRef>(ex, std::bind_front(Recheck, std::ref(sessions))));
        jsonrpc.Register("torrents.remove",           { Kind::User, Kind::Machine }, MakeMethod<RemoveReq>(ex, std::bind_front(Remove, std::ref(sessions))));
        jsonrpc.Register("torrents.resume",           { Kind::User, Kind::Machine }, MakeMethod<TorrentRef>(ex, std::bind_front(Resume, std::ref(sessions))));
        jsonrpc.Register("torrents.trackers.add",     { Kind::User, Kind::Machine }, MakeMethod<TrackersAddReq>(ex, std::bind_front(TrackersAdd, std::ref(sessions))));
        jsonrpc.Register("torrents.trackers.list",    { Kind::User, Kind::Machine }, MakeMethod<TorrentRef>(ex, std::bind_front(TrackersList, std::ref(sessions))));
    }
}
