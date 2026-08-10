#pragma once

#include <filesystem>
#include <memory>
#include <optional>

#include <boost/signals2.hpp>
#include <sqlite3.h>

#include "method.hpp"
#include "mmdblookup_reqres.hpp"

namespace porla
{
    class Mmdb;
}

namespace porla::Methods
{
    class MmdbLookup : public Method<MmdbLookupReq, MmdbLookupRes>
    {
    public:
        explicit MmdbLookup(sqlite3* db, boost::signals2::signal<void(const std::unordered_set<std::string>&)>& kv_updated);

    protected:
        void Invoke(const MmdbLookupReq& req, WriteCb<MmdbLookupRes> cb) override;

    private:
        struct State;
        std::shared_ptr<State> m_state;
    };
}
