#pragma once

#include <sqlite3.h>

#include "method.hpp"
#include "torrentslist_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    template <bool SSL> class TorrentsList : public Method<TorrentsListReq, TorrentsListRes, SSL>
    {
    public:
        explicit TorrentsList(porla::Sessions& sessions);

        void Invoke(const TorrentsListReq& req, WriteCb<TorrentsListRes, SSL> cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
