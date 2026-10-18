#include "presets.hpp"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>

#include "../functionmethod.hpp"
#include "../jsonrpc.hpp"

#include "../../json/lttypes.hpp"
#include "../../json/utils.hpp"
#include "../../presets.hpp"

using boost::asio::awaitable;

namespace porla::Rpc::Methods::Presets
{
    namespace
    {
        struct AddReq
        {
            std::string name;
        };

        struct GetReq
        {
            int id;
        };

        struct GetRes
        {
            struct Preset
            {
                int                                                    id;
                std::string                                            name;
                bool                                                   is_default;
                std::map<std::string, nlohmann::json>                  metadata;

                std::optional<std::string>                             category;
                std::optional<int>                                     download_limit;
                std::optional<lt::torrent_flags_t>                     flags;
                std::optional<lt::torrent_flags_t>                     flags_mask;
                std::optional<int>                                     max_connections;
                std::optional<int>                                     max_uploads;
                std::optional<std::string>                             save_path;
                std::optional<int>                                     session_id;
                std::optional<std::string>                             storage_mode;
                std::unordered_set<std::string>                        tags;
                std::optional<int>                                     upload_limit;
            };

            Preset preset;
        };

        struct ListReq {};

        struct ListRes
        {
            struct ListItem
            {
                int                                   id;
                std::string                           name;
                bool                                  is_default;
                std::map<std::string, nlohmann::json> metadata;
            };

            std::vector<ListItem> presets;
        };

        struct RemoveReq
        {
            int id;
        };

        struct UpdateReq
        {
            int                                                  id;
            std::string                                          name;
            std::optional<bool>                                  is_default;
            std::optional<std::string>                           category;
            std::optional<int>                                   download_limit;
            std::optional<lt::torrent_flags_t>                   flags;
            std::optional<lt::torrent_flags_t>                   flags_mask;
            std::optional<int>                                   max_connections;
            std::optional<int>                                   max_uploads;
            std::optional<std::map<std::string, nlohmann::json>> metadata;
            std::optional<int>                                   session_id;
            std::optional<std::string>                           save_path;
            std::optional<std::string>                           storage_mode;
            std::unordered_set<std::string>                      tags;
            std::optional<int>                                   upload_limit;
        };

        PORLA_JSON_REQUEST(AddReq, name)
        PORLA_JSON_REQUEST(GetReq, id)
        PORLA_JSON_NO_PARAMS(ListReq)
        PORLA_JSON_REQUEST(RemoveReq, id)

        PORLA_JSON_REQUEST(
            UpdateReq,
            id,
            name,
            is_default,
            category,
            download_limit,
            flags,
            flags_mask,
            max_connections,
            max_uploads,
            metadata,
            session_id,
            save_path,
            storage_mode,
            tags,
            upload_limit)

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
            GetRes::Preset,
            id,
            name,
            is_default,
            metadata,
            category,
            download_limit,
            flags,
            flags_mask,
            max_connections,
            max_uploads,
            save_path,
            session_id,
            storage_mode,
            tags,
            upload_limit)

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
            GetRes,
            preset)

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
            ListRes::ListItem,
            id,
            name,
            is_default,
            metadata)

        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_ONLY_SERIALIZE(
            ListRes,
            presets)

        awaitable<nlohmann::json> Add(porla::Presets& presets, const AddReq& req)
        {
            co_return nlohmann::json({
                {"id", presets.Add(req.name)}
            });
        }

        awaitable<GetRes> Get(porla::Presets& presets, const GetReq &req)
        {
            const auto preset = presets.Get(req.id);

            if (!preset.has_value())
            {
                throw Error(-1, "Preset not found");
            }

            co_return GetRes{
                .preset = GetRes::Preset{
                    .id = preset->id,
                    .name = preset->name,
                    .is_default = preset->is_default,
                    .metadata = preset->metadata.has_value()
                        ? preset->metadata.value()
                        : std::map<std::string, nlohmann::json>(),
                    .category = preset->category,
                    .download_limit = preset->download_limit,
                    .flags = preset->flags,
                    .flags_mask = preset->flags_mask,
                    .max_connections = preset->max_connections,
                    .max_uploads = preset->max_uploads,
                    .save_path = preset->save_path,
                    .session_id = preset->session_id,
                    .storage_mode = preset->storage_mode,
                    .tags = preset->tags,
                    .upload_limit = preset->upload_limit
                }
            };
        }

        awaitable<ListRes> List(porla::Presets& presets, const ListReq& req)
        {
            std::vector<ListRes::ListItem> result;

            for (const auto& preset : presets.All())
            {
                result.emplace_back(ListRes::ListItem{
                    .id         = preset.id,
                    .name       = preset.name,
                    .is_default = preset.is_default,
                    .metadata   = preset.metadata.has_value()
                        ? preset.metadata.value()
                        : std::map<std::string, nlohmann::json>()
                });
            }

            co_return ListRes{
                .presets = result
            };
        }

        awaitable<void> Remove(porla::Presets& presets, const RemoveReq& req)
        {
            const auto preset = presets.Get(req.id);

            if (!preset.has_value())
            {
                throw Error(-1, "Preset not found");
            }

            presets.Remove(preset->id);

            co_return;
        }

        awaitable<void> Update(porla::Presets& presets, const UpdateReq& req)
        {
            const auto preset = presets.Get(req.id);

            if (!preset.has_value())
            {
                throw Error(-1, "Preset not found", {{"id", req.id}});
            }

            if (req.flags.has_value() != req.flags_mask.has_value())
            {
                throw Error(-2, "'flags' and 'flags_mask' must be set together");
            }

            presets.Update(porla::Presets::Preset{
                .id = req.id,
                .name = req.name,
                .is_default = req.is_default.value_or(preset->is_default),
                .category = req.category,
                .download_limit = req.download_limit,
                .flags = req.flags,
                .flags_mask = req.flags_mask,
                .max_connections = req.max_connections,
                .max_uploads = req.max_uploads,
                .metadata = req.metadata,
                .session_id = req.session_id,
                .save_path = req.save_path,
                .storage_mode = req.storage_mode,
                .tags = req.tags,
                .upload_limit = req.upload_limit
            });

            co_return;
        }
    }

    void Register(boost::asio::io_context& io, JsonRpc& jsonrpc, porla::Presets& presets)
    {
        using Kind = porla::Auth::Context::Kind;

        auto ex = io.get_executor();

        jsonrpc.Register("presets.add",    { Kind::User                }, MakeMethod<AddReq>(ex, std::bind_front(Add, std::ref(presets))));
        jsonrpc.Register("presets.get",    { Kind::User, Kind::Machine }, MakeMethod<GetReq>(ex, std::bind_front(Get, std::ref(presets))));
        jsonrpc.Register("presets.list",   { Kind::User, Kind::Machine }, MakeMethod<ListReq>(ex, std::bind_front(List, std::ref(presets))));
        jsonrpc.Register("presets.remove", { Kind::User,               }, MakeMethod<RemoveReq>(ex, std::bind_front(Remove, std::ref(presets))));
        jsonrpc.Register("presets.update", { Kind::User,               }, MakeMethod<UpdateReq>(ex, std::bind_front(Update, std::ref(presets))));
    }
}