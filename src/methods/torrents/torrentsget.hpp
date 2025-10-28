#pragma once

#include "../method.hpp"
#include "torrentsget_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsGet : public Method<TorrentsGetReq, TorrentsGetRes>
    {
    public:
        explicit TorrentsGet(porla::Sessions& sessions);

    protected:
        void Invoke(const TorrentsGetReq& req, WriteCb<TorrentsGetRes> cb) override;

    private:
        porla::Sessions& m_sessions;
    };
}
