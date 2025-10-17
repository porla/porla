#include "mmdblookup.hpp"

#include "../data/models/keyvaluestore.hpp"
#include "../mmdb.hpp"

using porla::Data::Models::KeyValueStore;
using porla::Methods::MmdbLookup;
using porla::Methods::MmdbLookupReq;
using porla::Methods::MmdbLookupRes;

struct MmdbLookup::State
{
    std::unique_ptr<porla::Mmdb> mmdb;
};

MmdbLookup::MmdbLookup(std::optional<fs::path> file)
{
    m_state = std::make_shared<MmdbLookup::State>();

    if (file.has_value())
    {
        m_state->mmdb = Mmdb::Load(file.value());
    }
}

void MmdbLookup::Invoke(const MmdbLookupReq& req, WriteCb<MmdbLookupRes> cb)
{
    if (m_state->mmdb == nullptr)
    {
        return cb.Error(-1, "MMDB not loaded");
    }

    std::map<std::string, json> results;

    for (const auto& value : req.values)
    {
        results.insert({ value, m_state->mmdb->Lookup(value) });
    }

    cb.Ok(MmdbLookupRes{
        .results = results
    });
}
