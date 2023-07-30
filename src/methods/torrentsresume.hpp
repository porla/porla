#pragma once

#include "method.hpp"
#include "torrentsresume_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    class TorrentsResume : public Method<TorrentsResumeReq, TorrentsResumeRes>
    {
    public:
        explicit TorrentsResume(Sessions& sessions);

    protected:
        void Invoke(const TorrentsResumeReq& req, WriteCb<TorrentsResumeRes> cb) override;

    private:
        Sessions& m_sessions;
    };
}
