#include "fields.hpp"

#include "torrentclientdata.hpp"
#include "utils/eta.hpp"
#include "utils/ratio.hpp"

using porla::Fields;

namespace
{
    std::optional<double> Seconds(std::chrono::seconds s)
    {
        return static_cast<double>(s.count());
    }

    std::optional<double> SecondsSince(lt::time_point tp)
    {
        if (tp.time_since_epoch().count() == 0) { return std::nullopt; }
        return static_cast<double>(std::max<std::int64_t>(0, lt::total_seconds(lt::clock_type::now() - tp)));
    }

    std::optional<double> Timestamp(std::time_t t)
    {
        if (t <= 0) { return std::nullopt; }
        return static_cast<double>(t);
    }
}

const std::vector<Fields::Field>& Fields::All()
{
    static const std::vector<Field> fields =
    {
        { "errc",                   Kind::Bool, nullptr, nullptr, [](const Context& ctx) { return std::optional<bool>(ctx.status.errc); } },
        // error_file,
        { "save_path",              Kind::Text, nullptr, [](const Context& ctx) { return std::optional<std::string_view>(ctx.status.save_path); } },
        { "name",                   Kind::Text, nullptr, [](const Context& ctx) { return std::optional<std::string_view>(ctx.status.name); } },
        // torrent_file
        { "next_announce",          Kind::Duration, [](const Context& ctx) -> std::optional<double>
            {
                const auto seconds = lt::total_seconds(ctx.status.next_announce);
                if (seconds <= 0) { return std::nullopt; }
                return static_cast<double>(seconds);
            }
        },
        { "current_tracker",        Kind::Text, nullptr, [](const Context& ctx) { return std::optional<std::string_view>(ctx.status.current_tracker); } },
        { "total_download",         Kind::Size, [](const Context& ctx) { return std::optional<double>(ctx.status.total_download); } },
        { "total_upload",           Kind::Size, [](const Context& ctx) { return std::optional<double>(ctx.status.total_upload); } },
        { "total_payload_download", Kind::Size, [](const Context& ctx) { return std::optional<double>(ctx.status.total_payload_download); } },
        { "total_payload_upload",   Kind::Size, [](const Context& ctx) { return std::optional<double>(ctx.status.total_payload_upload); } },
        { "total_failed_bytes",     Kind::Size, [](const Context& ctx) { return std::optional<double>(ctx.status.total_failed_bytes); } },
        { "total_redundant_bytes",  Kind::Size, [](const Context& ctx) { return std::optional<double>(ctx.status.total_redundant_bytes); } },
        // pieces
        // verified_pieces
        { "total_done",             Kind::Size, [](const Context& ctx) { return std::optional<double>(ctx.status.total_done); } },
        { "total",                  Kind::Size, [](const Context& ctx) { return std::optional<double>(ctx.status.total); } },
        { "total_wanted_done",      Kind::Size, [](const Context& ctx) { return std::optional<double>(ctx.status.total_wanted_done); } },
        { "total_wanted",           Kind::Size, [](const Context& ctx) { return std::optional<double>(ctx.status.total_wanted); } },
        { "total_wanted_remaining", Kind::Size, [](const Context& ctx) { return std::optional<double>(ctx.status.total_wanted - ctx.status.total_wanted_done); } },
        { "all_time_upload",        Kind::Size, [](const Context& ctx) { return std::optional<double>(ctx.status.all_time_upload); } },
        { "all_time_download",      Kind::Size, [](const Context& ctx) { return std::optional<double>(ctx.status.all_time_download); } },
        { "added_time",             Kind::Date, [](const Context& ctx) { return Timestamp(ctx.status.added_time); } },
        { "completed_time",         Kind::Date, [](const Context& ctx) { return Timestamp(ctx.status.completed_time); } },
        { "last_seen_complete",     Kind::Date, [](const Context& ctx) { return Timestamp(ctx.status.last_seen_complete); } },
        // { "storage_mode",           Kind::Date, [](const Context& ctx) { return std::optional<double>(ctx.status.last_seen_complete); } },
        { "progress",               Kind::Percent, [](const Context& ctx) { return std::optional<double>(ctx.status.progress_ppm); } },
        { "queue_position",         Kind::Number, [](const Context& ctx) -> std::optional<double> { const int queue_pos = static_cast<int>(ctx.status.queue_position); if (queue_pos < 0) { return std::nullopt; } return queue_pos; } },
        { "download_rate",          Kind::Rate, [](const Context& ctx) { return std::optional<double>(ctx.status.download_rate); } },
        { "upload_rate",            Kind::Rate, [](const Context& ctx) { return std::optional<double>(ctx.status.upload_rate); } },
        { "download_payload_rate",  Kind::Rate, [](const Context& ctx) { return std::optional<double>(ctx.status.download_payload_rate); } },
        { "upload_payload_rate",    Kind::Rate, [](const Context& ctx) { return std::optional<double>(ctx.status.upload_payload_rate); } },
        { "num_seeds",              Kind::Number, [](const Context& ctx) { return std::optional<double>(ctx.status.num_seeds); } },
        { "num_peers",              Kind::Number, [](const Context& ctx) { return std::optional<double>(ctx.status.num_peers); } },
        { "num_complete",           Kind::Number, [](const Context& ctx) -> std::optional<double> { if (ctx.status.num_complete < 0) { return std::nullopt; } return ctx.status.num_complete; } },
        { "num_incomplete",         Kind::Number, [](const Context& ctx) -> std::optional<double> { if (ctx.status.num_incomplete < 0) { return std::nullopt; } return ctx.status.num_incomplete; } },
        { "list_seeds",             Kind::Number, [](const Context& ctx) { return std::optional<double>(ctx.status.list_seeds); } },
        { "list_peers",             Kind::Number, [](const Context& ctx) { return std::optional<double>(ctx.status.list_peers); } },
        { "connect_candidates",     Kind::Number, [](const Context& ctx) { return std::optional<double>(ctx.status.connect_candidates); } },
        { "num_pieces",             Kind::Number, [](const Context& ctx) { return std::optional<double>(ctx.status.num_pieces); } },
        // distributed full copies
        // distributed fraction
        // distributed_copies
        { "block_size",             Kind::Number, [](const Context& ctx) { return std::optional<double>(ctx.status.block_size); } },
        { "num_uploads",            Kind::Number, [](const Context& ctx) { return std::optional<double>(ctx.status.num_uploads); } },
        { "num_connections",        Kind::Number, [](const Context& ctx) { return std::optional<double>(ctx.status.num_connections); } },
        { "uploads_limit",          Kind::Number, [](const Context& ctx) -> std::optional<double> { if (ctx.status.uploads_limit < 0) { return std::nullopt; } return ctx.status.uploads_limit; } },
        { "connections_limit",      Kind::Number, [](const Context& ctx) -> std::optional<double> { if (ctx.status.connections_limit < 0) { return std::nullopt; } return ctx.status.connections_limit; } },
        { "upload_limit",           Kind::Rate,   [](const Context& ctx) -> std::optional<double> { if (ctx.status.upload_limit < 0) { return std::nullopt; } return ctx.status.upload_limit; } },
        { "download_limit",         Kind::Rate,   [](const Context& ctx) -> std::optional<double> { if (ctx.status.download_limit < 0) { return std::nullopt; } return ctx.status.download_limit; } },
        { "up_bandwidth_queue",     Kind::Number, [](const Context& ctx) { return std::optional<double>(ctx.status.up_bandwidth_queue); } },
        { "down_bandwidth_queue",   Kind::Number, [](const Context& ctx) { return std::optional<double>(ctx.status.down_bandwidth_queue); } },
        { "seed_rank",              Kind::Number, [](const Context& ctx) { return std::optional<double>(ctx.status.seed_rank); } },
        { "state",                  Kind::State },
        // need_save_resume_data
        { "is_seeding",             Kind::Bool, nullptr, nullptr, [](const Context& ctx) { return std::optional<bool>(ctx.status.is_seeding); } },
        { "is_finished",            Kind::Bool, nullptr, nullptr, [](const Context& ctx) { return std::optional<bool>(ctx.status.is_finished); } },
        { "has_metadata",           Kind::Bool, nullptr, nullptr, [](const Context& ctx) { return std::optional<bool>(ctx.status.has_metadata); } },
        { "has_incoming",           Kind::Bool, nullptr, nullptr, [](const Context& ctx) { return std::optional<bool>(ctx.status.has_incoming); } },
        { "moving_storage",         Kind::Bool, nullptr, nullptr, [](const Context& ctx) { return std::optional<bool>(ctx.status.moving_storage); } },
        { "announcing_to_trackers", Kind::Bool, nullptr, nullptr, [](const Context& ctx) { return std::optional<bool>(ctx.status.announcing_to_trackers); } },
        { "announcing_to_lsd",      Kind::Bool, nullptr, nullptr, [](const Context& ctx) { return std::optional<bool>(ctx.status.announcing_to_lsd); } },
        { "announcing_to_dht",      Kind::Bool, nullptr, nullptr, [](const Context& ctx) { return std::optional<bool>(ctx.status.announcing_to_dht); } },
        { "info_hash",              Kind::Hash },
        { "last_upload",            Kind::Duration, [](const Context& ctx) { return SecondsSince(ctx.status.last_upload); } },
        { "last_download",          Kind::Duration, [](const Context& ctx) { return SecondsSince(ctx.status.last_download); } },
        { "active_duration",        Kind::Duration, [](const Context& ctx) { return Seconds(ctx.status.active_duration); } },
        { "finished_duration",      Kind::Duration, [](const Context& ctx) { return Seconds(ctx.status.finished_duration); } },
        { "seeding_duration",       Kind::Duration, [](const Context& ctx) { return Seconds(ctx.status.seeding_duration); } },
        { "flags",                  Kind::Flags },

        // porla fields
        { "eta", Kind::Duration, [](const Context& ctx) -> std::optional<double>
        {
            const auto eta = Utils::ETA(ctx.status).count();
            if (eta < 0) { return std::nullopt; }
            return static_cast<double>(eta);
        }},

        { "ratio", Kind::Number, [](const Context& ctx) -> std::optional<double>
        {
            return Utils::Ratio(ctx.status);
        }},

        { "ratio_real", Kind::Number, [](const Context& ctx) -> std::optional<double>
        {
            return Utils::RealRatio(ctx.status);
        }},

        { "$userdata.category", Kind::Text, nullptr, [](const Context& ctx) -> std::optional<std::string_view>
        {
            if (ctx.client_data == nullptr || !ctx.client_data->category.has_value())
            {
                return std::nullopt;
            }

            return ctx.client_data->category.value();
        }},

        { "$userdata.tags", Kind::Tag }
    };

    return fields;
}

const Fields::Field* Fields::Find(std::string_view name)
{
    for (const auto& field : All())
    {
        if (field.name == name)
        {
            return &field;
        }
    }

    return nullptr;
}
