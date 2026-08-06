#pragma once

#include <sqlite3.h>

#include "../method.hpp"
#include "torrentsremovereq.hpp"
#include "torrentsremoveres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsRemove : public Method<TorrentsRemoveReq, TorrentsRemoveRes>
    {
    public:
        explicit TorrentsRemove(sqlite3* db, porla::Sessions& sessions);

    protected:
        void Invoke(const TorrentsRemoveReq& req, WriteCb<TorrentsRemoveRes> cb) override;

    private:
        sqlite3* m_db;
        porla::Sessions& m_sessions;
    };
}
