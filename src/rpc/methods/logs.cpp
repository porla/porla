#include "logs.hpp"

#include <boost/asio/awaitable.hpp>

#include "../functionmethod.hpp"
#include "../jsonrpc.hpp"

#include "../../json/utils.hpp"
#include "../../logger.hpp"

using boost::asio::awaitable;

namespace porla::Rpc::Methods::Logs
{
    namespace
    {
        struct ListFilters
        {
            std::optional<std::string> component;
            std::optional<std::string> min_level;
            std::optional<int>         plugin_id;
        };

        struct ListReq
        {
            std::optional<std::uint64_t> after;
            std::optional<ListFilters>   filters;
            std::optional<int>           limit;
        };

        struct ListRes
        {
            struct Entry
            {
                std::uint64_t              seq;
                std::int64_t               ts;
                std::string                level;
                std::optional<std::string> component;
                nlohmann::json             properties;
                std::string                message;
            };

            std::vector<Entry> entries;
            std::uint64_t      cursor;
            std::string        run_id;
        };

        PORLA_JSON_REQUEST(ListFilters, component, min_level, plugin_id)
        PORLA_JSON_REQUEST(ListReq, after, filters, limit)

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(ListRes::Entry, seq, ts, level, component, properties, message)
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(ListRes, entries, cursor, run_id)

        Error InvalidField(const std::string& field, const std::string& reason)
        {
            return Error(Errors::Code::invalid_params, "Invalid " + field + ": " + reason, {{"field", field}});
        }

        awaitable<ListRes> List(const Log::Buffer& logs, const ListReq& req)
        {
            Log::Query query{ .after = req.after };

            if (req.limit.has_value())
            {
                if (*req.limit < 1 || *req.limit > 1000)
                {
                    throw InvalidField("limit", "must be between 1 and 1000");
                }

                query.limit = static_cast<std::size_t>(*req.limit);
            }

            if (req.filters.has_value())
            {
                const auto& filters = *req.filters;

                if (filters.min_level.has_value()
                    && !boost::log::trivial::from_string(filters.min_level->data(), filters.min_level->size(), query.min_level))
                {
                    throw InvalidField("filters.min_level", "must be one of trace, debug, info, warning, error, fatal");
                }

                query.component = filters.component;
                query.plugin_id = filters.plugin_id;
            }

            auto page = logs.List(query);

            ListRes res{ .entries = {}, .cursor = page.cursor, .run_id = page.run_id };
            res.entries.reserve(page.entries.size());

            for (auto& entry : page.entries)
            {
                auto properties = nlohmann::json::object();

                for (auto& [name, value] : entry.properties)
                {
                    std::visit(
                        [&](auto& v)
                        {
                            if constexpr (std::same_as<std::decay_t<decltype(v)>, std::monostate>)
                            {
                                properties[name] = nullptr;
                            }
                            else
                            {
                                properties[name] = std::move(v);
                            }
                        },
                        value);
                }

                res.entries.push_back(ListRes::Entry{
                    .seq        = entry.seq,
                    .ts         = entry.ts,
                    .level      = boost::log::trivial::to_string(entry.level),
                    .component  = std::move(entry.component),
                    .properties = std::move(properties),
                    .message    = std::move(entry.message)
                });
            }

            co_return res;
        }
    }

    void Register(boost::asio::io_context& io, JsonRpc& jsonrpc, const Log::Buffer& logs)
    {
        using Kind = Auth::Context::Kind;

        jsonrpc.Register("logs.list", { Kind::User }, MakeMethod<ListReq>(io.get_executor(), std::bind_front(List, std::cref(logs))));
    }
}
