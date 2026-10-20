#include "mmdb.hpp"

#include <filesystem>
#include <map>
#include <string>
#include <unordered_set>

#include <boost/asio/awaitable.hpp>
#include <boost/log/trivial.hpp>
#include <boost/signals2.hpp>
#include <nlohmann/json.hpp>

#include "../functionmethod.hpp"
#include "../jsonrpc.hpp"

#include "../../events.hpp"
#include "../../json/utils.hpp"
#include "../../keyvalue.hpp"
#include "../../mmdb.hpp"

namespace fs = std::filesystem;

namespace porla::Rpc::Methods::Mmdb
{
    namespace
    {
        struct LookupReq
        {
            std::unordered_set<std::string> values;
        };

        PORLA_JSON_REQUEST(LookupReq, values)

        struct State
        {
            explicit State(KeyValue& kv) : kv(kv)
            {
            }

            void Load()
            {
                mmdb = nullptr;

                const auto mmdb_path = kv.Get("porla.mmdb.path");

                if (mmdb_path.is_string() && mmdb_path != "")
                {
                    std::error_code ec;
                    if (!fs::exists(mmdb_path.get<std::string>(), ec))
                    {
                        BOOST_LOG_TRIVIAL(error) << "MMDB path " << mmdb_path.get<std::string>() << " is not usable: "
                                                << (ec ? ec.message() : "does not exist");
                        return;
                    }

                    BOOST_LOG_TRIVIAL(info) << "Loading MMDB file from " << mmdb_path.get<std::string>();

                    mmdb = porla::Mmdb::Load(mmdb_path.get<std::string>());
                }
            }

            porla::KeyValue&                   kv;
            std::unique_ptr<porla::Mmdb>       mmdb;
            boost::signals2::scoped_connection reload;
        };

        boost::asio::awaitable<nlohmann::json> Lookup(const std::shared_ptr<State>& state, const LookupReq& req)
        {
            if (state->mmdb == nullptr)
            {
                throw Error(-1, "MMDB not loaded");
            }

            std::map<std::string, nlohmann::json> results;

            for (const auto& value : req.values)
            {
                results.insert({ value, state->mmdb->Lookup(value) });
            }

            co_return nlohmann::json({
                {"results", results}
            });
        }

    }

    void Register(boost::asio::io_context& io, JsonRpc& jsonrpc, KeyValue& kv, Events& events)
    {
        using Kind = porla::Auth::Context::Kind;

        auto state = std::make_shared<State>(kv);
        state->reload = events.On("kv.updated", [weak = std::weak_ptr(state)](const Event& event)
        {
            const auto& keys = event.data.at("keys");

            if (auto s = weak.lock(); s && std::ranges::find(keys, "porla.mmdb.path") != keys.end())
            {
                BOOST_LOG_TRIVIAL(debug) << "Reloading MMDB file";
                s->Load();
            }
        });

        state->Load();

        jsonrpc.Register("mmdb.lookup", { Kind::Machine, Kind::User }, MakeMethod<LookupReq>(io.get_executor(), std::bind_front(Lookup, state)));
    }
}
