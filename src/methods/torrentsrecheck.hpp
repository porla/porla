#pragma once

#include "method.hpp"
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
        explicit TorrentsRecheck(Sessions& sessions);

    protected:
        void Invoke(const TorrentsRecheckReq& req, WriteCb<TorrentsRecheckRes> cb) override;

    private:
        Sessions& m_sessions;
    };
}
