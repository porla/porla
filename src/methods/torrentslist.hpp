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
    class TorrentsList : public Method<TorrentsListReq, TorrentsListRes>
    {
    public:
        explicit TorrentsList(porla::Sessions& sessions);

        void Invoke(const TorrentsListReq& req, WriteCb<TorrentsListRes> cb) override;

    private:
        void InvokeImpl(const TorrentsListReq& req, WriteCb<TorrentsListRes>& cb);

    private:
        porla::Sessions& m_sessions;
    };
}
