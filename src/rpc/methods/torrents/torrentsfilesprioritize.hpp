#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "torrentsfilesprioritize_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsFilesPrioritize : public TypedMethod<TorrentsFilesPrioritizeReq, TorrentsFilesPrioritizeRes>
    {
    public:
        explicit TorrentsFilesPrioritize(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsFilesPrioritizeReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
