#include "mmdblookup.hpp"

#include <boost/log/trivial.hpp>

#include "../../../data/models/keyvaluestore.hpp"
#include "../../../mmdb.hpp"

namespace fs = std::filesystem;

using json = nlohmann::json;

using porla::Data::Models::KeyValueStore;
using porla::Rpc::Methods::Mmdb::MmdbLookup;
using porla::Rpc::Methods::Mmdb::MmdbLookupReq;
using porla::Rpc::Methods::Mmdb::MmdbLookupRes;

struct MmdbLookup::State
{
    sqlite3*                     db;
    std::unique_ptr<porla::Mmdb> mmdb;
    boost::signals2::connection  reload;

    void Load()
    {
        mmdb = nullptr;

        const auto mmdb_path = Data::Models::KeyValueStore::Get(db, "porla.mmdb.path");

        if (mmdb_path.is_string() && mmdb_path != "")
        {
            if (!fs::exists(mmdb_path))
            {
                BOOST_LOG_TRIVIAL(error) << "MMDB path " << mmdb_path.get<std::string>() << " does not exist";
                return;
            }

            BOOST_LOG_TRIVIAL(info) << "Loading MMDB file from " << mmdb_path.get<std::string>();

            mmdb = porla::Mmdb::Load(mmdb_path.get<std::string>());
        }
    }
};

MmdbLookup::MmdbLookup(sqlite3* db, boost::signals2::signal<void(const std::unordered_set<std::string>&)>& kv_updated)
{
    m_state = std::make_shared<MmdbLookup::State>();
    m_state->db = db;
    m_state->reload = kv_updated.connect([s = m_state](const std::unordered_set<std::string>& keys)
    {
        if (keys.contains("porla.mmdb.path"))
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
