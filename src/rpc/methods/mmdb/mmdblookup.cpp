#include "mmdblookup.hpp"

#include <boost/log/trivial.hpp>

#include "../../../events.hpp"
#include "../../../keyvalue.hpp"
#include "../../../mmdb.hpp"

namespace fs = std::filesystem;

using json = nlohmann::json;

using porla::Events;
using porla::KeyValue;
using porla::Rpc::Methods::Mmdb::MmdbLookup;
using porla::Rpc::Methods::Mmdb::MmdbLookupReq;
using porla::Rpc::Methods::Mmdb::MmdbLookupRes;

struct MmdbLookup::State
{
    explicit State(KeyValue& kv)
        : kv(kv)
    {
    }

    porla::KeyValue&                   kv;
    std::unique_ptr<porla::Mmdb>       mmdb;
    boost::signals2::scoped_connection reload;

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
};

MmdbLookup::MmdbLookup(KeyValue& kv, Events& events)
{
    m_state = std::make_shared<MmdbLookup::State>(kv);
    m_state->reload = events.On("kv.updated", [weak = std::weak_ptr(m_state)](const porla::Event& event)
    {
        const auto& keys = static_cast<const porla::KeyValueEvent&>(event).keys;

        if (auto s = weak.lock(); s && std::ranges::find(keys, "porla.mmdb.path") != keys.end())
        {
            BOOST_LOG_TRIVIAL(debug) << "Reloading MMDB file";
            s->Load();
        }
    });

    m_state->Load();
}

void MmdbLookup::Execute(const MmdbLookupReq& req, ResponseWriterHandle cb)
{
    if (m_state->mmdb == nullptr)
    {
        return cb->Error(-1, "MMDB not loaded");
    }

    std::map<std::string, json> results;

    for (const auto& value : req.values)
    {
        results.insert({ value, m_state->mmdb->Lookup(value) });
    }

    cb->Ok(MmdbLookupRes{
        .results = results
    });
}
