#include "kv.hpp"

#include "../functionmethod.hpp"
#include "../jsonrpc.hpp"

#include "../../json/utils.hpp"
#include "../../keyvalue.hpp"

namespace porla::Rpc::Methods::Kv
{
    namespace
    {
        struct KeyValueGetReq
        {
            std::unordered_set<std::string> keys;
        };

        struct KeyValueSetReq
        {
            std::map<std::string, nlohmann::json> values;
        };

        boost::asio::awaitable<nlohmann::json> Get(const KeyValue& kv, const KeyValueGetReq& req)
        {
            std::map<std::string, nlohmann::json> values;

            for (const auto& key : req.keys)
            {
                values.insert({ key, kv.Get(key) });
            }

            co_return nlohmann::json({
                {"values", values}
            });
        }

        boost::asio::awaitable<void> Set(KeyValue& kv, const KeyValueSetReq& req)
        {
            kv.Set(req.values);
            co_return;
        }

        PORLA_JSON_REQUEST(KeyValueGetReq, keys)
        PORLA_JSON_REQUEST(KeyValueSetReq, values)
    }

    void Register(boost::asio::io_context& io, JsonRpc& jsonrpc, KeyValue& kv)
    {
        using Kind = Auth::Context::Kind;

        jsonrpc.Register("kv.get", { Kind::User, Kind::Machine }, MakeMethod<KeyValueGetReq>(io.get_executor(), std::bind_front(Get, std::cref(kv))));
        jsonrpc.Register("kv.set", { Kind::User },                MakeMethod<KeyValueSetReq>(io.get_executor(), std::bind_front(Set, std::ref(kv))));
    }
}
