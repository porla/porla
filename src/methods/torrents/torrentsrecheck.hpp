#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "torrentsrecheck_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsRecheck : public Method<TorrentsRecheckReq, TorrentsRecheckRes>
    {
    public:
        explicit TorrentsRecheck(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Invoke(const TorrentsRecheckReq& req, WriteCb<TorrentsRecheckRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
