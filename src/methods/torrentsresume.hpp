#pragma once

#include "method.hpp"
#include "torrentsresume_reqres.hpp"

namespace porla
{
    class Sessions;
}

namespace porla::Methods
{
    template <bool SSL> class TorrentsResume : public Method<TorrentsResumeReq, TorrentsResumeRes, SSL>
    {
    public:
        explicit TorrentsResume(Sessions& sessions);

    protected:
        void Invoke(const TorrentsResumeReq& req, WriteCb<TorrentsResumeRes, SSL> cb) override;

    private:
        Sessions& m_sessions;
    };
}
