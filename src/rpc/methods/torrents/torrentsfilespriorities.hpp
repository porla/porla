#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "torrentsfilespriorities_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsFilesPriorities : public TypedMethod<TorrentsFilesPrioritiesReq, TorrentsFilesPrioritiesRes>
    {
    public:
        explicit TorrentsFilesPriorities(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsFilesPrioritiesReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
