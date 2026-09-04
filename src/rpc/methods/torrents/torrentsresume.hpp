#pragma once

#include <sqlite3.h>

#include "../../typedmethod.hpp"
#include "torrentsresume_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Rpc::Methods::Torrents
{
    class TorrentsResume : public TypedMethod<TorrentsResumeReq, TorrentsResumeRes>
    {
    public:
        explicit TorrentsResume(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Execute(const TorrentsResumeReq& req, ResponseWriterHandle cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
