#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "torrentsfilesprogress_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsFilesProgress : public TypedMethod<TorrentsFilesProgressReq, TorrentsFilesProgressRes>
    {
    public:
        explicit TorrentsFilesProgress(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsFilesProgressReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
