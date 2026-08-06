#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "torrentsfilesprogress_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsFilesProgress : public Method<TorrentsFilesProgressReq, TorrentsFilesProgressRes>
    {
    public:
        explicit TorrentsFilesProgress(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Invoke(const TorrentsFilesProgressReq& req, WriteCb<TorrentsFilesProgressRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
